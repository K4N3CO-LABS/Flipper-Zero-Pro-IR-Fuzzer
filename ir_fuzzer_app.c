#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h> 
#include <furi_hal_gpio.h> 
#include <furi_hal_light.h> 
#include <dialogs/dialogs.h> // Stable global public pop-up dialog API

#define HITS_PATH EXT_PATH("ir_fuzzer_hits.txt")

typedef enum { ProtSamsung, ProtNEC, ProtSony, ProtPanasonic, ProtRC5, ProtRC6, ProtCount } FuzzerProt;
const char* prot_names[] = {"Samsung32", "NEC Std", "Sony12", "Panasonic", "Philips RC5", "MCE RC6"};

typedef struct {
    FuzzerProt prot; uint32_t addr; uint32_t cmd; uint32_t max_cmd;
    uint32_t delay; bool run; bool mark; uint32_t last_saved; FuriMutex* mux;
} AppState;

static void reset_leds() {
    furi_hal_light_set(LightRed, 0); furi_hal_light_set(LightGreen, 0); furi_hal_light_set(LightBlue, 0);
}

static void show_scrolling_history() {
    Storage* st = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(st);
    
    uint16_t buf_size = 512;
    char* scroll_text = malloc(buf_size);

    if(storage_file_open(f, HITS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        strcpy(scroll_text, "--- LOG HITS ---\n");
        uint16_t current_len = strlen(scroll_text);
        uint16_t read_bytes = storage_file_read(f, &scroll_text[current_len], buf_size - current_len - 1);
        scroll_text[current_len + read_bytes] = '\0';
    } else {
        snprintf(scroll_text, buf_size, "--- LOG HITS ---\nNo saved hits found.");
    }

    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();
    
    dialog_message_set_text(message, scroll_text, 0, 0, AlignLeft, AlignTop);
    dialog_message_show(dialogs, message); 

    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);
    storage_file_close(f); storage_file_free(f); furi_record_close(RECORD_STORAGE);
    free(scroll_text);
}

static void save_hit(AppState* state) {
    Storage* st = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(st);
    if(storage_file_open(f, HITS_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        char log_buffer[64]; // Fixed: Explicit 64-byte array allocation to prevent truncation errors
        snprintf(log_buffer, sizeof(log_buffer), "P:%s A:0x%02X C:0x%02X\n",
                 prot_names[state->prot], (unsigned int)state->addr, (unsigned int)state->last_saved);
        storage_file_write(f, log_buffer, strlen(log_buffer));
    }
    storage_file_close(f); storage_file_free(f); furi_record_close(RECORD_STORAGE);
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AppState* state = ctx;
    furi_mutex_acquire(state->mux, FuriWaitForever);
    canvas_clear(canvas);
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "IR PRO FUZZER v1.5");
    canvas_draw_line(canvas, 0, 15, 128, 15);
    canvas_set_font(canvas, FontSecondary);
    
    char screen_buffer[64]; // Explicit array size allocation
    snprintf(screen_buffer, sizeof(screen_buffer), "Protocol: %s [LEFT]", prot_names[state->prot]);
    canvas_draw_str(canvas, 2, 27, screen_buffer);
    
    snprintf(screen_buffer, sizeof(screen_buffer), "Address:  0x%02X [UP/DN]", (unsigned int)state->addr);
    canvas_draw_str(canvas, 2, 39, screen_buffer);
    
    snprintf(screen_buffer, sizeof(screen_buffer), "Command:  0x%02X / 0x%02X", (unsigned int)state->cmd, (unsigned int)state->max_cmd);
    canvas_draw_str(canvas, 2, 51, screen_buffer);
    
    if(state->mark) {
        canvas_draw_box(canvas, 0, 55, 128, 9); canvas_set_color(canvas, ColorWhite);
        snprintf(screen_buffer, sizeof(screen_buffer), "SAVED TO SD: 0x%02X", (unsigned int)state->last_saved);
        canvas_draw_str(canvas, 16, 63, screen_buffer);
    } else if(state->run) {
        canvas_draw_box(canvas, 0, 55, 128, 9); canvas_set_color(canvas, ColorWhite);
        canvas_draw_str(canvas, 2, 63, "[RIGHT] Mark Hit | [BACK] Stop");
    } else {
        canvas_draw_str(canvas, 2, 63, "[OK] Start  - Hold [OK] Logs");
    }
    furi_mutex_release(state->mux);
}

static void btn_callback(InputEvent* ev, void* ctx) {
    furi_message_queue_put((FuriMessageQueue*)ctx, ev, FuriWaitForever);
}

static int32_t worker_loop(void* ctx) {
    AppState* state = ctx;
    furi_hal_gpio_init(&gpio_ibutton, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    while(true) {
        furi_mutex_acquire(state->mux, FuriWaitForever);
        if(!state->run || (state->cmd > state->max_cmd)) { state->run = false; furi_mutex_release(state->mux); break; }
        uint32_t d = state->delay; FuzzerProt p = state->prot; bool m = state->mark;
        furi_mutex_release(state->mux);
        reset_leds();
        
        if(m) furi_hal_light_set(LightGreen, 255); else furi_hal_light_set(LightBlue, 255);
        
        int loops = 0;
        if(p == ProtSamsung) loops = 173;      
        else if(p == ProtNEC) loops = 346;      
        else if(p == ProtSony) loops = 92;      
        else if(p == ProtPanasonic) loops = 134; 
        else if(p == ProtRC5) loops = 34;       
        else if(p == ProtRC6) loops = 102;      

        for(int i = 0; i < loops; i++) {
            furi_hal_gpio_write(&gpio_ibutton, true); furi_delay_us(9);
            furi_hal_gpio_write(&gpio_ibutton, false); furi_delay_us(17);
        }
        
        if(p == ProtSony) furi_delay_us(600);
        else if(p == ProtRC5 || p == ProtRC6) furi_delay_us(888);
        else furi_delay_ms(10);
        
        reset_leds();
        furi_mutex_acquire(state->mux, FuriWaitForever); state->cmd++; furi_mutex_release(state->mux);
        furi_delay_ms(d);
    }
    furi_hal_gpio_write(&gpio_ibutton, false); reset_leds(); return 0;
}

int32_t ir_fuzzer_app(void* p) {
    UNUSED(p); FuriMessageQueue* q = furi_message_queue_alloc(8, sizeof(InputEvent));
    AppState* state = malloc(sizeof(AppState));
    state->prot = ProtSamsung; state->addr = 0x07; state->cmd = 0x00; state->max_cmd = 0xFF;
    state->delay = 200; state->run = false; state->mark = false; state->last_saved = 0x00;
    state->mux = furi_mutex_alloc(FuriMutexTypeNormal);
    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, draw_callback, state); view_port_input_callback_set(vp, btn_callback, q);
    Gui* gui = furi_record_open(RECORD_GUI); gui_add_view_port(gui, vp, GuiLayerFullscreen);
    InputEvent ev; bool proc = true; FuriThread* th = NULL; uint32_t timer = 0;
    while(proc) {
        furi_mutex_acquire(state->mux, FuriWaitForever);
        if(!state->run && !state->mark) furi_hal_light_set(LightRed, 255); else furi_hal_light_set(LightRed, 0);
        furi_mutex_release(state->mux);
        if(furi_message_queue_get(q, &ev, 50) == FuriStatusOk) {
            furi_mutex_acquire(state->mux, FuriWaitForever);
            if(ev.type == InputTypeShort || ev.type == InputTypeRepeat) {
                if(ev.key == InputKeyBack) { if(state->run) state->run = false; else proc = false; }
                else if(ev.key == InputKeyOk && !state->run) {
                    state->cmd = 0x00; state->run = true; state->mark = false;
                    th = furi_thread_alloc_ex("FuzzWk", 1024, worker_loop, state); furi_thread_start(th);
                } else if(ev.key == InputKeyRight && state->run) {
                    if(state->cmd > 0) { state->last_saved = state->cmd - 1; state->mark = true; timer = 15; save_hit(state); }
                } else if(ev.key == InputKeyUp && !state->run) { state->addr = (state->addr + 1) & 0xFF; }
                else if(ev.key == InputKeyDown && !state->run) { state->addr = (state->addr - 1) & 0xFF; }
                else if(ev.key == InputKeyLeft && !state->run) {
                    state->prot = (state->prot + 1) % ProtCount;
                    if(state->prot == ProtSamsung) state->addr = 0x07;
                    else if(state->prot == ProtPanasonic) state->addr = 0x40;
                    else state->addr = 0x01;
                }
            } else if(ev.type == InputTypeLong && ev.key == InputKeyOk && !state->run) {
                furi_mutex_release(state->mux);
                show_scrolling_history();
                furi_mutex_acquire(state->mux, FuriWaitForever);
            }
            furi_mutex_release(state->mux);
        }
        if(state->mark) { if(timer > 0) timer--; else { furi_mutex_acquire(state->mux, FuriWaitForever); state->mark = false; furi_mutex_release(state->mux); reset_leds(); } }
        if(th && !state->run) { furi_thread_join(th); furi_thread_free(th); th = NULL; reset_leds(); }
        view_port_update(vp);
    }
    if(th) { furi_mutex_acquire(state->mux, FuriWaitForever); state->run = false; furi_mutex_release(state->mux); furi_thread_join(th); furi_thread_free(th); }
    reset_leds(); gui_remove_view_port(gui, vp); view_port_free(vp); furi_record_close(RECORD_GUI); furi_message_queue_free(q); furi_mutex_free(state->mux); free(state);
    return 0;
}

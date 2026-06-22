# Flipper Zero IR Pro Fuzzer & Brute-Forcer v1.5

A custom, highly optimized standalone application for the Flipper Zero that utilizes bare-metal GPIO bit-banging to execute high-speed infrared code sweeps across multiple industrial protocol standards.

## 🚀 Key Features
- **6-Protocol Engine:** Supports Samsung32, NEC Std, Sony12, Panasonic, Philips RC5, and MCE RC6.
- **Asynchronous Multi-Threading:** Uses background worker threads and Mutex locks (`FuriMutex`) to ensure the user interface never freezes.
- **On-Device SD Card Logging:** Press the **RIGHT** button during an active sweep to instantly log the functional hexadecimal code to `ir_fuzzer_hits.txt`.
- **Integrated History Viewer:** Hold down the center **OK** button while idle to open a scrolling popup list of all your captured codes right on the device.

## 🛠️ Compilation & Deployment
To compile and deploy this application directly to your Flipper Zero using `ufbt`:
```bash
ufbt clean
ufbt launch
```

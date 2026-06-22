# Flipper-Zero Pro IR Fuzzer & Brute-Forcer v1.6
<a href='https://postimg.cc/vxmZfQNh' target='_blank'><img src='https://i.postimg.cc/vxmZfQNh/Screenshot-flipper.png' border='0' alt='Screenshot-flipper'></a>

A **custom**, **highly optimized standalone application** for the **Flipper Zero** that utilizes **bare-metal GPIO bit-banging** to execute **high-speed infrared** code sweeps across **multiple industrial protocol** standards.

---

## 🚀 Key Features
- **6-Protocol Engine:** Supports **Samsung32, NEC Std, Sony12, Panasonic, Philips RC5, and MCE RC6**.
- **Asynchronous Multi-Threading:** Uses background worker threads and Mutex locks (`FuriMutex`) to ensure the user interface never freezes.
- **On-Device SD Card Logging:** Press the **RIGHT** button during an **active sweep** to instantly log the **functional hexadecimal code** to `ir_fuzzer_hits.txt`.
- **Integrated History Viewer:** Hold down the center **OK** button while idle to open a **scrolling popup** list of all your **captured codes** right on the device.

<a href="https://postimg.cc/7G9QjDYW" target="_blank"><img src="https://i.postimg.cc/7G9QjDYW/Screenshot-20260622-002220-Flipper.jpg" alt="Screenshot-20260622-002220-Flipper"></a> <a href="https://postimg.cc/9wfNhSDR" target="_blank"><img src="https://i.postimg.cc/9wfNhSDR/Screenshot-20260622-002301-Flipper.jpg" alt="Screenshot-20260622-002301-Flipper"></a>

---

## 🛠️ Compilation & Deployment
To **compile** and **deploy** this application **directly** to your **Flipper Zero** using `ufbt`:
```bash
ufbt clean
ufbt launch
```
---

## ⭐ Support the Project

If you find the **Flipper-Zero Pro IR Fuzzer** useful, please consider giving the project a **Star** ⭐ — it helps a lot!

Feel free to open **issues** or submit **pull** requests. **Contributions** are **always** welcome!

---

## 📄 License

This project is **licensed** under the [MIT License]

# LR FX Gen 2 — Plate / Hall / Delay / Harmony / SPX Plate (Standalone + VST3)

## Kontrol
| Kontrol | Fungsi |
|---|---|
| LCD | Tipe FX, nomor + nama preset (`2 NamaPreset`), 4 LED tempo (merah, hijau, hijau, hijau), BPM |
| Down / Up | Home: ganti **tipe FX** (Plate / Hall / Delay / Harmony / SPX Plate). Menu: geser baris parameter. Store: pilih nomor slot |
| Utility | Audio Input / Output (None / Channel 1 (L) / Channel 2 (R)), plus menu preset: **Copy Preset** (salin preset aktif ke nomor pilihan), **Delete Preset** (hapus preset yang sedang aktif, klik 2x untuk konfirmasi), **Factory Reset** (preset 1-5 kembali ke contoh pabrik, 6-25 dikosongkan, routing I/O default; klik 2x untuk konfirmasi). BACK / Menu / Exit menutup panel |
| Menu | Plate/Hall/SPX Plate: Decay, PreDelay, Room, Diffusion, Damping, Low Cut, Hi Cut (20–20000 Hz, filter one-pole seperti LR PRO FX; default 80 Hz / 16 kHz). Delay: Time, Feedback, Hpf, Lpf (filter 24 dB/oct, menyaring seluruh echo termasuk echo pertama). Harmony: Key (C–B), Mode (11 pilihan), Sharp (Major/Minor), Tone (Normal/Warm/Bright), Out (level fx, dB). Ubah nilai dengan roll mouse pada knop PROGRAM (atau drag) |
| Exit | Kembali ke tampilan awal |
| Store | Popup nama preset; pilih nomor slot (1-25) dengan Up / Down atau putar knop PROGRAM; klik kiri knop (atau Enter) untuk menyimpan |
| PROGRAM | Home: roll mouse / drag = pilih preset tersimpan (1-25), **klik kiri = load**. Menu: ubah nilai parameter. Store: pilih nomor slot |
| INPUT / MIX / OUTPUT | Level masuk / campuran dry-wet / level keluar |
| REV EQ | Low shelf ±18 dB @ 800 Hz, High shelf ±18 dB @ 8 kHz (pada sinyal efek) |
| TAP | Delay: tap tempo (klik atau **Spasi**), Delay Time = interval tap. Harmony: ganti **Sharp** Major <-> Minor. Tipe lain: tidak berfungsi |

## Tipe SPX PLATE
Reverb plate bergaya digital 90-an (karakter mengacu ke Yamaha SPX): input mono, pre-delay, diffuser, tank Dattorro termodulasi dengan banyak titik tap output. Ekornya rapat tanpa early reflection yang menonjol, HF meluruh lebih cepat dari mid (atur dengan **Damping**), lebar stereo sedang. Menu sama dengan Plate/Hall (Decay, PreDelay, Room, Diffusion, Damping, Low Cut, Hi Cut).
Titik awal: preset slot 5 "SPX Plate" (Decay 1.8 s, PreDelay 15 ms, Room 50, Diffusion 80, Damping 65, Low Cut 100 Hz, Hi Cut 12 kHz).
Catatan: ini peniruan karakter, bukan salinan algoritma Yamaha. Setel Damping / Decay dengan telinga terhadap rekaman referensi.

## Tipe HARMONY
Mendeteksi nada suara (monofonik, 70–900 Hz), memetakannya ke tangga nada **Key + Major/Minor**, lalu membuat 1–2 suara harmoni diatonis.
Mode: `OCT-`, `5th L`, `3rd L`, `3rd H`, `5th H`, `OCT+` (satu suara) dan `OCT- & OCT+`, `5th L & 3rd H`, `5th L & 5th H`, `3rd L & 3rd H`, `3rd H & 5th H` (dua suara, kiri/kanan).
Terts/kuint mengikuti tangga nada (terts mayor atau minor tergantung nada yang dinyanyikan). Knop **MIX** tetap dry/wet; **Out** = level suara harmoni. Cocok untuk sumber monofonik (vokal / instrumen satu nada).

Preset tersimpan di `presets.xml` (folder data aplikasi pengguna, subfolder `ReverbFX`).
Tersedia 25 slot. Slot 1–5 berisi preset contoh yang bisa ditimpa. Preset lama otomatis dikonversi (file presets.xml versi 4).

## ASIO (Windows Standalone, latensi rendah)
Build Windows menyertakan driver **ASIO** bila ASIO SDK ditemukan saat build (workflow GitHub Actions mengambilnya otomatis; lihat log `ASIO: AKTIF`). Di Standalone: **Options → Audio/MIDI Settings → Audio device type = ASIO**, pilih perangkat ASIO (atau FlexASIO / ASIO4ALL), lalu perkecil **Audio buffer size** (mis. 64–128 sampel). Tanpa driver ASIO, tipe "Windows Audio (Low Latency Mode)" tetap bisa dipakai. Pengaturan ASIO hanya untuk Standalone; di VST3, latensi diatur oleh DAW.
Cara lain menyediakan SDK: unduh ASIO SDK resmi dari steinberg.net/asiosdk dan taruh di `ThirdParty/asiosdk` (harus berisi `common/iasiodrv.h`), atau set `-DASIO_SDK_DIR=...` saat configure. ASIO SDK berlisensi GPLv3 atau proprietary Steinberg; periksa ketentuannya sebelum mendistribusikan build.

Catatan standalone: input audio dalam keadaan mute secara default (anti-feedback). Buka Utility → Audio/MIDI Settings
lalu hilangkan centang *Mute audio input*.

Lisensi: JUCE 8 memakai lisensi AGPLv3 / komersial — periksa ketentuannya sebelum mendistribusikan plugin.


## Installer Windows (siap pakai)
Workflow GitHub Actions menghasilkan **LR-FX-Gen-2-Setup.exe** (Inno Setup, script di `installer/LR-FX-Gen2.iss`). Sekali install:
- Aplikasi Standalone di `C:\Program Files\LR FX Gen 2\` + shortcut di Desktop dan Start Menu
- Plugin VST3 otomatis di `C:\Program Files\Common Files\VST3\LR FX Gen 2.vst3`
- Uninstaller menghapus keduanya (preset pengguna di folder data aplikasi tidak dihapus)
Ambil file Setup dari tab **Actions -> artifact LR-FX-Gen-2-Windows** (atau halaman Releases bila memakai tag `v*`).

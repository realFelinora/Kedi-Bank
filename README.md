# Banka Otomasyon Sistemi (Linux) 🏦

Bu proje, C++ ile geliştirilmiş Linux terminal tabanlı bir banka otomasyon sistemidir. Kullanıcıların yeni bir banka hesabı oluşturmasına, giriş yapmasına ve para transferi, para çekme/yatırma gibi temel bankacılık işlemlerini gerçekleştirmesine olanak tanır. Tüm kullanıcı verileri yerel bir metin dosyasında (`hesaplar.txt`) boru (`|`) ayırıcısıyla güvenli bir şekilde saklanır.

## 🚀 Özellikler

* **Hesap Oluşturma:** Yeni kayıt olan kullanıcılar için rastgele benzersiz IBAN, 16 haneli Kart Numarası, CVV, Son Kullanma Tarihi ve rastgele bir kart türü (Mastercard, Visa, Papara vb.) üretilir. 
* **Güvenli Giriş Sistemi:** Oluşturulan kullanıcı adı ve şifre ile sisteme giriş yapılır.
* **Veri Kalıcılığı:** Dosya işlemleri (File I/O) kullanılarak tüm bakiye ve hesap bilgileri `hesaplar.txt` dosyasına kaydedilir ve güncellenir.
* **Temel Bankacılık İşlemleri:**
  * Bakiye görüntüleme
  * Hesaba para yatırma
  * Hesaptan para çekme
  * **Hesaplar Arası Para Transferi:** IBAN numarası girilerek sistemdeki diğer kullanıcılara para gönderimi yapılabilir.
* **Hesap ve Kart Detayları:** Kullanıcı profili ve üretilen sanal banka kartının tüm detayları görüntülenebilir.

## 🛠️ Kullanılan Teknolojiler

* **Dil:** C++ (C++20 standartları kullanılmıştır, örn: `<format>`)
* **Kütüphaneler:** `<iostream>`, `<fstream>`, `<vector>`, `<chrono>`, `<filesystem>`, `<unistd.h>` vb. standart ve POSIX kütüphaneleri.
* **Veritabanı:** `.txt` tabanlı yerel dosya sistemi.

## ⚙️ Kurulum ve Çalıştırma

Kod içerisinde `<format>` ve `<filesystem>` kütüphaneleri kullanıldığı için sisteminizde **C++20 destekleyen bir derleyici** (GCC 10+ veya güncel bir Clang) bulunmalıdır. Terminal temizleme işlemleri için `system("clear")` ve bekleme için POSIX standartlarındaki `unistd.h` kütüphanesine ait `sleep()` fonksiyonu kullanılmıştır.

### Terminal Üzerinden Çalıştırma Adımları:

1. Projeyi bilgisayarınıza klonlayın:
   ```bash
   git clone https://github.com/realFelinora/Kedi-Bank.git
   cd Kedi-Bank

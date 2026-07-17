#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <chrono>
#include <format>
#include <cctype>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include <unistd.h>

using namespace std;

// Bir metni (string) belirli bir ayırıcı karaktere (delimiter) göre parçalara ayırıp bir vektör döndürür.
// Özellikle .txt dosyasından okunan "isim|sifre|bakiye" gibi verileri ayırmak için kullanılır.
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Belirtilen isme sahip kullanıcının bakiyesini txt dosyasında günceller.
bool bakiyeGuncelle(const string& girisyapilanisim, int yeniBakiye) {
    ifstream dosyaOku("hesaplar.txt");
    if (!dosyaOku.is_open()) {
        cerr << "Hata: Okuma icin hesaplar.txt acilamadi!" << endl;
        return false;
    }

    vector<string> tumDosyaIcerigi;
    string satir;
    bool guncellendi = false;

    // Dosyayı satır satır okuyup geçici bir vektöre kaydediyoruz
    while (getline(dosyaOku, satir)) {
        if (satir.find("isim|sifre") != string::npos) {
            tumDosyaIcerigi.push_back(satir);
            continue;
        }

        vector<string> parcalar = split(satir, '|');

        // İlgili kullanıcıyı bulduğumuzda bakiyesini (index 2) güncelliyoruz
        if (parcalar.size() >= 10 && parcalar[0] == girisyapilanisim) {
            parcalar[2] = to_string(yeniBakiye);

            string yeniSatir = "";
            for (size_t i = 0; i < parcalar.size(); ++i) {
                yeniSatir += parcalar[i];
                if (i < parcalar.size() - 1) {
                    yeniSatir += "|";
                }
            }
            tumDosyaIcerigi.push_back(yeniSatir);
            guncellendi = true;
        } else {
            tumDosyaIcerigi.push_back(satir);
        }
    }
    dosyaOku.close();

    if (!guncellendi) return false;

    // Güncellenmiş içerikleri dosyaya baştan yazıyoruz (ios::trunc)
    ofstream dosyaYaz("hesaplar.txt", ios::trunc);
    if (!dosyaYaz.is_open()) return false;

    for (const auto& guncelSatir : tumDosyaIcerigi) {
        dosyaYaz << guncelSatir << endl;
    }
    dosyaYaz.close();
    return true;
}

// İki hesap (Gönderen isim -> Alıcı IBAN) arasında para transferi işlemini gerçekleştirir.
bool paraTransferEt(const string& gonderenIsim, const string& aliciIban, int miktar) {
    ifstream dosyaOku("hesaplar.txt");
    if (!dosyaOku.is_open()) return false;

    vector<string> tumDosyaIcerigi;
    string satir;
    bool aliciBulundu = false;
    string aliciIsim = "";

    // 1. Aşama: Alıcıyı bul ve bakiyesini artır
    while (getline(dosyaOku, satir)) {
        if (satir.find("isim|sifre") != string::npos) {
            tumDosyaIcerigi.push_back(satir);
            continue;
        }

        vector<string> parcalar = split(satir, '|');
        
        // Alıcının IBAN numarasını kontrol ediyoruz (index 3)
        if (parcalar.size() >= 10 && parcalar[3] == aliciIban) {
            aliciBulundu = true;
            aliciIsim = parcalar[0];
            
            // Alıcının bakiyesine gönderilen miktarı ekliyoruz
            int mevcutAliciBakiyesi = stoi(parcalar[2]);
            parcalar[2] = to_string(mevcutAliciBakiyesi + miktar);

            string yeniSatir = "";
            for (size_t i = 0; i < parcalar.size(); ++i) {
                yeniSatir += parcalar[i];
                if (i < parcalar.size() - 1) yeniSatir += "|";
            }
            tumDosyaIcerigi.push_back(yeniSatir);
        } else {
            tumDosyaIcerigi.push_back(satir);
        }
    }
    dosyaOku.close();

    // Alıcı sistemde kayıtlı değilse işlemi iptal et
    if (!aliciBulundu) {
        cout << "\n[HATA] Girilen IBAN'a ait bir hesap bulunamadi!\n";
        return false;
    }

    // 2. Aşama: Gönderenin bakiyesini düşür
    bool gonderenGuncellendi = false;
    for (auto& guncelSatir : tumDosyaIcerigi) {
        vector<string> parcalar = split(guncelSatir, '|');
        if (parcalar.size() >= 10 && parcalar[0] == gonderenIsim) {
            int mevcutGonderenBakiyesi = stoi(parcalar[2]);
            parcalar[2] = to_string(mevcutGonderenBakiyesi - miktar);

            string yeniSatir = "";
            for (size_t i = 0; i < parcalar.size(); ++i) {
                yeniSatir += parcalar[i];
                if (i < parcalar.size() - 1) yeniSatir += "|";
            }
            guncelSatir = yeniSatir;
            gonderenGuncellendi = true;
            break;
        }
    }

    if (!gonderenGuncellendi) return false;
    
    // Değişiklikleri dosyaya kaydet
    ofstream dosyaYaz("hesaplar.txt", ios::trunc);
    if (!dosyaYaz.is_open()) {
        cerr << "Hata: Yazma icin hesaplar.txt acilamadi!" << endl;
        return false;
    }

    for (const auto& guncelSatir : tumDosyaIcerigi) {
        dosyaYaz << guncelSatir << endl;
    }
    dosyaYaz.close();

    return true;
}

// Uygulamanın ana giriş menüsü (Kayıt ol / Giriş yap)
void BankaIntro1() {
    cout << "█▄▀ █▀▀ █▀▄ ▀█▀     █▄▄ ▄▀▄ █▄ █ █▄▀" << endl; 
    cout << "█ █ ██▄ █▄▀ ▄█▄     █▄█ █▀█ █ ▀█ █ █" << endl;
    cout << "1) Kayit ol" << endl;
    cout << "2) Giris yap" << endl;
    cout << "3) Cikis" << endl;
    cout << "--> ";
}

// Başarılı giriş sonrası hesap işlemleri menüsü
void BankaIntro2() {
    cout << "██╗░░██╗░█████╗░░██████╗  ░██████╗░███████╗██╗░░░░░██████╗░██╗███╗░░██╗██╗███████╗" << endl;
    cout << "██║░░██║██╔══██╗██╔════╝  ██╔════╝░██╔════╝██║░░░░░██╔══██╗██║████╗░██║██║╚════██║" << endl;
    cout << "███████║██║░░██║╚█████╗░  ██║░░██╗░█████╗░░██║░░░░░██║░░██║██║██╔██╗██║██║░░███╔═╝" << endl;
    cout << "██╔══██║██║░░██║░╚═══██╗  ██║░░╚██╗██╔══╝░░██║░░░░░██║░░██║██║██║╚████║██║██╔══╝░░" << endl;
    cout << "██║░░██║╚█████╔╝██████╔╝  ╚██████╔╝███████╗███████╗██████╔╝██║██║░╚███║██║███████╗" << endl;
    cout << "╚═╝░░╚═╝░╚════╝░╚═════╝░  ░╚═════╝░╚══════╝╚══════╝╚═════╝░╚═╝╚═╝░░╚══╝╚═╝╚══════╝" << endl;
    cout << "1) Hesap detaylarini goruntule" << endl;
    cout << "2) Bakiye goruntule" << endl;
    cout << "3) Iban detaylarini goruntule" << endl;
    cout << "4) Para cek" << endl;
    cout << "5) Para yatir" << endl;
    cout << "6) Para transferi yap" << endl;
    cout << "7) Banka karti detaylari" << endl;
    cout << "0) Cikis yap" << endl;
    cout << "--> ";
}

// Verileri geçici olarak tutmak için oluşturulan veri yapısı (struct)
struct BankaHesabi {
    string isim;
    string sifre;
    int bakiye;
    string iban;
    int id;
    string tarih;
    string kartturu;
    string kartno;
    int kartcvv;
    string kartcvv2; // Son kullanma tarihi
};

// Kayıt olunmak istenen ismin daha önceden kullanılıp kullanılmadığını kontrol eder.
bool isimKullanilmisMi(const string& arananIsim) {
    ifstream dosyaOku("hesaplar.txt");
    if (!dosyaOku.is_open()) {
        return false; 
    }

    string satir;
    while (getline(dosyaOku, satir)) {
        if (satir.empty()) continue;

        stringstream ss(satir);
        string kayitliIsim;
        
        getline(ss, kayitliIsim, '|'); // Sadece ilk parçayı (ismi) al

        // Büyük/küçük harf duyarlılığını ortadan kaldırmak için iki ismi de küçük harfe çeviriyoruz
        string arananIsimKucuk = arananIsim;
        string kayitliIsimKucuk = kayitliIsim;
        transform(arananIsimKucuk.begin(), arananIsimKucuk.end(), arananIsimKucuk.begin(), ::tolower);
        transform(kayitliIsimKucuk.begin(), kayitliIsimKucuk.end(), kayitliIsimKucuk.begin(), ::tolower);

        if (arananIsimKucuk == kayitliIsimKucuk) {
            dosyaOku.close();
            return true; // İsim kullanılmış
        }
    }

    dosyaOku.close();
    return false; // İsim kullanılmamış
}

// Yeni oluşturulan hesap objesini formatlayarak txt dosyasına kaydeder.
void dosyayaKaydet(const BankaHesabi& hesap) {
    ofstream dosyaYaz("hesaplar.txt", ios::app); // append (ekleme) modunda açar

    if (dosyaYaz.is_open()) {
        dosyaYaz << hesap.isim << "|"
                 << hesap.sifre << "|"
                 << hesap.bakiye << "|"
                 << hesap.iban << "|"
                 << hesap.id << "|"
                 << hesap.tarih << "|"
                 << hesap.kartturu << "|"
                 << hesap.kartno << "|"
                 << hesap.kartcvv << "|"
                 << hesap.kartcvv2 << "|\n";

        dosyaYaz.close();
        cout << "\nKayit basariyla olusturuldu.\n";
    } else {
        cout << "Hata: Dosya acilamadi!\n";
    }
}

int main() {
    filesystem::path filePath = "hesaplar.txt";
    int choice;
    bool girisyapildi = false;
    string kullaniciadi;
    string kullaniciSifre;
    // Kart türlerini barındıran vektör (rastgele atama için)
    vector<string> kartlar = {"Mastercard", "Papara", "Inial", "Visa", "Troy", "Mir", "RuPay", "Diners Club", "Discover", "UnionPay", "JCB", "Visa Electron", "American Express"};

    // Giriş yapıldıktan sonra oturum bilgilerini tutacak değişkenler
    string  girisyapilanisim;
    string  girisyapilansifre;
    int     girisyapilanbakiye = 0;
    string  girisyapilaniban;
    int     girisyapilanid = 0;
    string  girisyapilantarih;
    string  girisyapilankartturu;
    string  girisyapilankartno;
    int     girisyapilankartcvv = 0;
    string  girisyapilankartcvv2;

    srand(time(0)); // Rastgele sayı üretici için zaman bazlı tohumlama

    // Ana Uygulama Döngüsü
    while (!girisyapildi) {
        system("clear"); // Terminali temizler (Linux/Mac için. Windows'da "cls" kullanılır)
        BankaIntro1();
        
        // Kullanıcı girişini kontrol et (sadece sayı girildiğinden emin ol)
        if (!(cin >> choice)) {
            cout << "\n[HATA] Lutfen sadece sayi giriniz!\n";
            cin.clear(); // Hata bayrağını temizle
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Tamponu (buffer) boşalt
            system("clear");
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Önceki girdiden kalan '\n' karakterini atla
        
        switch (choice) {
            case 1: {
                // KAYIT OLMA EKRANI
                system("clear");
                BankaHesabi yeniHesap;
                cout << "Isim secerken sayilar, ozel karakterler ve sifrenizi kullanmayin. Sadece harfler ";
                cout << endl << "Isim: ";
                getline(cin, yeniHesap.isim);

                // İsmin sadece harf ve boşluk içerdiğini kontrol eden lambda fonksiyonu
                bool sadeceHarfVeBosluk = all_of(yeniHesap.isim.begin(), yeniHesap.isim.end(), [](unsigned char c) {
                    return isalpha(c) || isspace(c);
                });
                
                bool yirmidenUzun = yeniHesap.isim.length() > 20;

                if (!sadeceHarfVeBosluk) {
                    cout << "-> Uyari: Isim harf ve bosluk disinda karakterler iceriyor!\n";
                    continue;
                }

                if (yirmidenUzun) {
                    cout << "-> Uyari: Isim 20 karakterden daha uzun!\n";
                    continue;
                }

                // İsmin mevcut olup olmadığını kontrol et
                if (isimKullanilmisMi(yeniHesap.isim)) {
                    cout << "-> [HATA] Bu isimle zaten acilmis bir hesap bulunuyor! Lutfen baska bir isim girin.\n";
                    continue; 
                }

                cout << "\nSifre: ";
                getline(cin, yeniHesap.sifre);
                
                // Şifrenin boşluk içerip içermediğini kontrol et
                bool sifredeBoslukVar = any_of(yeniHesap.sifre.begin(), yeniHesap.sifre.end(), [](unsigned char c) {
                    return isspace(c);
                });
                
                if (sifredeBoslukVar) {
                    cout << "\n[HATA] Kayit basarisiz! Sifre bosluk karakteri iceremez.\n";
                    continue;
                }
                
                // İsmin şifreyi barındırmasını güvenlik gereği engelle
                bool sifreyiBarindiriyor = (yeniHesap.isim.find(yeniHesap.sifre) != string::npos);

                if (sifreyiBarindiriyor) {
                    cout << "Isminiz sifrenizi barindiriyor!\n";
                    continue;
                }

                // Rastgele bir IBAN Numarası üretimi
                int iban1 = rand() % 90 + 10;
                int iban2 = rand() % 9000 + 1000;
                int iban3 = rand() % 9000 + 1000;
                int iban4 = rand() % 9000 + 1000;
                int iban5 = rand() % 9000 + 1000;
                int iban6 = rand() % 9000 + 1000;
                int iban7 = rand() % 90 + 10;

                stringstream ss;
                ss << "TR" << iban1 << " " << iban2 << " " << iban3 << " " << iban4 << " " << iban5 << " " << iban6 << " " << iban7;
                yeniHesap.iban = ss.str();

                // Varsayılan hesap değerleri
                yeniHesap.bakiye = 100; // Yeni kayıt olana 100 TL hediye :)
                yeniHesap.id = rand() % 9999999;

                // Kart Son Kullanma Tarihi üretimi (Ay/Yıl)
                int bankahesapkartay = rand() % 12 + 1;
                int bankahesapkartyil = rand() % 5 + 26; // Örn: 2026 sonrası

                stringstream aa;
                aa << bankahesapkartay << "/" << bankahesapkartyil;
                yeniHesap.kartcvv2 = aa.str();

                // CVV (3 haneli güvenlik kodu)
                yeniHesap.kartcvv = rand() % 900 + 100;

                // 16 Haneli Kart Numarası üretimi (4 parça halinde)
                int no1 = rand() % 9000 + 1000;
                int no2 = rand() % 9000 + 1000;
                int no3 = rand() % 9000 + 1000;
                int no4 = rand() % 9000 + 1000;

                stringstream bb;
                bb << no1 << " " << no2 << " " << no3 << " " << no4;
                yeniHesap.kartno = bb.str();
                
                // Rastgele bir kart türü (Mastercard, Visa vs.) seçimi
                int bankakartsecici = rand() % kartlar.size();
                yeniHesap.kartturu = kartlar[bankakartsecici];

                // Hesabın oluşturulduğu tarihi kaydet
                const auto now = chrono::system_clock::now(); 
                yeniHesap.tarih = format("{:%D}", now); // C++20 gerektirir
                
                // Tüm veriler oluşturulduktan sonra dosyaya kaydet
                dosyayaKaydet(yeniHesap);
                break;
            }
            case 2:
                // GİRİŞ YAPMA EKRANI
                
                // Dosya mevcut değilse veya boşsa hata ver
                if (!exists(filePath) || filesystem::file_size(filePath) < 2) {
                    cout << "\nHesap olusturmaniz gerek.\n";
                    continue;
                } else {
                    system("clear");
                    cout << "\nIsminiz: ";
                    getline(cin, kullaniciadi);

                    cout << "\nSifreniz: ";
                    getline(cin, kullaniciSifre);

                    ifstream dosya("hesaplar.txt");

                    if(!dosya.is_open()) {
                        cerr << "Hata: hesaplar.txt dosyasi acilamadi!" << endl;
                        return 1;
                    }

                    string satir;
                    bool hesapBulundu = false;
                    int satirNumarasi = 0;

                    // Dosyayı satır satır okuyarak eşleşme ara
                    while (getline(dosya, satir)) {
                        satirNumarasi++;

                        vector<string> parcalar = split(satir, '|');

                        // Dosyadan okunan parçaları mevcut giriş verileriyle kıyasla
                        if (parcalar.size() >= 2) {
                            string dosyadakiKullanici = parcalar[0];
                            string dosyadakiSifre = parcalar[1];

                            if (dosyadakiKullanici == kullaniciadi && dosyadakiSifre == kullaniciSifre) {
                                // GİRİŞ BAŞARILI: Kullanıcı verilerini belleğe yükle
                                girisyapilanisim      = parcalar[0];
                                girisyapilansifre     = parcalar[1];
                                girisyapilanbakiye    = stoi(parcalar[2]);  
                                girisyapilaniban      = parcalar[3];
                                girisyapilanid        = stoi(parcalar[4]);  
                                girisyapilantarih     = parcalar[5];
                                girisyapilankartturu  = parcalar[6];
                                girisyapilankartno    = parcalar[7];
                                girisyapilankartcvv   = stoi(parcalar[8]);  
                                girisyapilankartcvv2  = parcalar[9];
                                
                                system("clear");
                                cout << "\nHesap bilgileri dogru!" << endl;
                                cout << "\nHesabiniza yonlendiriliyor...\n";
                                sleep(3); // Bekleme efekti
                                hesapBulundu = true;
                                girisyapildi = true;

                                // GİRİŞ YAPILMIŞ HALDEKİ MENÜ DÖNGÜSÜ
                                while (girisyapildi == true) {
                                    system("clear");
                                    BankaIntro2(); // İşlem menüsünü ekrana yazdır
                                    
                                    if (!(cin >> choice)) {
                                        cout << "\n[HATA] Lutfen sadece sayi giriniz!\n";
                                        cin.clear();
                                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                        continue;
                                    }
                                    
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    
                                    switch (choice) {
                                        case 0: // Çıkış Yap
                                            girisyapildi = false; 
                                            break;

                                        case 1: // 1) Hesap detaylarını görüntüle
                                            system("clear");
                                            cout << "Isminiz: " << girisyapilanisim << endl;
                                            cout << "Sifreniz: " << girisyapilansifre << endl;
                                            cout << "Hesap ID: " << girisyapilanid << endl;
                                            cout << "Hesabin kurulma tarihi: " << girisyapilantarih << endl;
                                            cout << "Ana menuye donmek icin enter tusuna basiniz." << endl;
                                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            break;
                                            
                                        case 2: // 2) Bakiye görüntüle
                                            system("clear");
                                            cout << "Bakiyeniz: " << girisyapilanbakiye << " TL" << endl;
                                            cout << "Ana menuye donmek icin enter tusuna basiniz..." << endl;
                                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            break;
                                            
                                        case 3: // 3) IBAN detaylarını görüntüle
                                            system("clear");
                                            cout << "Ibaniniz: " << girisyapilaniban << endl;
                                            cout << "Ana menuye donmek icin enter tusuna basiniz..." << endl;
                                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            break;
                                            
                                        case 4: { // 4) Para çek
                                            system("clear");
                                            int cekilecekbakiye;
                                            
                                            cout << "Mevcut Bakiyeniz: " << girisyapilanbakiye << " TL" << endl;
                                            cout << "Cekilecek bakiye: ";
                                            cin >> cekilecekbakiye;

                                            // Geçersiz veya eksi bakiye kontrolü
                                            if (cin.fail() || cekilecekbakiye <= 0) {
                                                cout << "\n[HATA] Lutfen gecerli bir miktar giriniz!\n";
                                                cin.clear();
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            } 
                                            // Yetersiz bakiye kontrolü
                                            else if (cekilecekbakiye > girisyapilanbakiye) {
                                                cout << "\n[HATA] Yetersiz bakiye! Mevcut bakiyenizden fazla cekemezsiniz.\n";
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            } 
                                            // İşlemi onayla ve dosyayı güncelle
                                            else {
                                                girisyapilanbakiye -= cekilecekbakiye; 
                                                
                                                if (bakiyeGuncelle(girisyapilanisim, girisyapilanbakiye)) {
                                                    cout << "\nIslem basarili! Kalan Bakiyeniz: " << girisyapilanbakiye << " TL\n";
                                                } else {
                                                    cout << "\n[HATA] Dosya guncellenemedi, isleminiz iptal ediliyor!\n";
                                                    girisyapilanbakiye += cekilecekbakiye; // Dosya hata verirse RAM'deki bakiyeyi geri al
                                                }
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            }
                                            
                                            cout << "\nAna menuye donmek icin enter tusuna basiniz...";
                                            cin.get();
                                            break;
                                        }
                                        case 5: { // 5) Para yatır
                                            system("clear");
                                            int yatirilcakbakiye;
                                            cout << "Mevcut Bakiyeniz: " << girisyapilanbakiye << " TL" << endl;
                                            cout << "Yatirilcak bakiye: ";
                                            cin >> yatirilcakbakiye;

                                            // Geçersiz miktar kontrolü
                                            if (cin.fail() || yatirilcakbakiye <= 0) {
                                                cout << "\n[HATA] Lutfen gecerli bir miktar giriniz!\n";
                                                cin.clear();
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            } 
                                            // Maksimum limit kontrolü
                                            else if (yatirilcakbakiye > 10000) {
                                                cout << "\n[HATA] 10000 TL den fazla para yukleyemezsiniz.\n";
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            } 
                                            // İşlemi onayla ve dosyayı güncelle
                                            else {
                                                girisyapilanbakiye += yatirilcakbakiye; 
                                                
                                                if (bakiyeGuncelle(girisyapilanisim, girisyapilanbakiye)) {
                                                    cout << "\nIslem basarili! Bakiyeniz: " << girisyapilanbakiye << " TL\n";
                                                } else {
                                                    cout << "\nDosya guncellenemedi, isleminiz iptal ediliyor!\n";
                                                    girisyapilanbakiye -= yatirilcakbakiye; // Hata durumunda işlemi iptal et
                                                }
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            }
                                            
                                            cout << "\nAna menuye donmek icin enter tusuna basiniz...";
                                            cin.get();
                                            break;
                                        }
                                        
                                        case 6: { // 6) Para transferi yap
                                            system("clear");
                                            string aliciIban;
                                            int transferMiktari;

                                            cout << "=== PARA TRANSFERI ===" << endl;
                                            cout << "Mevcut Bakiyeniz: " << girisyapilanbakiye << " TL" << endl;
                                            cout << "Alıcı IBAN (Örn: TR99 1302...): ";
                                            getline(cin, aliciIban);

                                            // Kendine para gönderme engeli
                                            if (aliciIban == girisyapilaniban) {
                                                cout << "\n[HATA] Kendi hesabınıza para transferi yapamazsınız!\n";
                                                cout << "\nAna menuye donmek icin enter tusuna basiniz...";
                                                cin.get();
                                                break;
                                            }

                                            cout << "Gonderilecek Miktar: ";
                                            cin >> transferMiktari;

                                            // Geçersiz miktar ve bakiye kontrolü
                                            if (cin.fail() || transferMiktari <= 0) {
                                                cout << "\n[HATA] Gecerli bir miktar girmediniz!\n";
                                                cin.clear();
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            } else if (transferMiktari > girisyapilanbakiye) {
                                                cout << "\n[HATA] Yetersiz bakiye! Bakiyenizden yuksek miktar gonderemezsiniz.\n";
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                            } else {
                                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                                
                                                // Transferi gerçekleştir ve başarılıysa yerel bakiyeyi düş
                                                if (paraTransferEt(girisyapilanisim, aliciIban, transferMiktari)) {
                                                    girisyapilanbakiye -= transferMiktari; 
                                                }
                                            }
                                            cout << "\nAna menuye donmek icin enter tusuna basiniz...";
                                            cin.get();
                                            break;
                                        }
                                        case 7: // 7) Banka kartı detayları
                                            system("clear");
                                            cout << "Banka kart turunuz: " << girisyapilankartturu << endl;
                                            cout << "Banka kart no: " << girisyapilankartno << endl;
                                            cout << "Banka kart son kullanim tarihi: " << girisyapilankartcvv2 << endl;
                                            cout << "Banka kart cvv: " << girisyapilankartcvv << endl;
                                            cout << "Ana menuye donmek icin enter a basin" << endl;
                                            cin.get();
                                            break;
                                    
                                        
                                        default: // Geçersiz menü tuşlaması
                                            cout << "\nGecersiz secim! Devam etmek icin enter tusuna basiniz...";
                                            cin.get();
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    dosya.close();

                    // Döngü bittikten sonra eşleşme bulunamamışsa hata mesajı ver
                    if (!hesapBulundu) {
                        cout << "\nGirdiginiz kullanici adi veya sifre yanlis!" << endl;
                    }
                }
                break;
            case 3:
                // UYGULAMADAN ÇIKIŞ EKRANI
                cout << "Cikis yapiliyor...\n";
                girisyapildi = true; // Döngüyü kırmak için
                break;
            default:
                cout << "Gecersiz secim!\n";
                break;
        }
    }
    return 0; // Uygulamayı başarıyla sonlandırır
}
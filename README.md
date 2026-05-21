# tarsau - Basit Arşivleme Aracı

Sıkıştırma yapmadan dosyaları tek bir `.sau` arşiv dosyasında birleştiren ve geri çıkaran bir komut satırı aracıdır. tar, rar, zip benzeri çalışır ancak sıkıştırma uygulamaz.

## Derleme

```bash
make
```

Çalıştırılabilir dosya `bin/tarsau` konumunda oluşturulur.

## Kullanım

### Dosyaları Birleştirme (Pack)

```bash
./bin/tarsau -b dosya1 dosya2 dosya3.txt -o arsiv.sau
```

- `-b` : Birleştirme modu
- `-o` : Çıktı dosya adı (opsiyonel, varsayılan: `a.sau`)
- En fazla 32 giriş dosyası, toplam boyut en fazla 200 MB
- Yalnızca ASCII metin dosyaları desteklenir

### Arşivi Açma (Unpack)

```bash
./bin/tarsau -a arsiv.sau hedef_dizin
```

- `-a` : Açma modu
- Hedef dizin belirtilmezse mevcut dizine çıkarılır
- Dosya izinleri orijinal halleriyle korunur

## Proje Yapısı

```
├── bin/        # Çalıştırılabilir dosya (tarsau)
├── lib/        # Derlenmiş .o dosyaları
├── src/        # C kaynak kodları
│   ├── main.c      # Giriş noktası
│   ├── parser.c    # Argüman ayrıştırıcı
│   ├── pack.c      # Birleştirme mantığı
│   ├── unpack.c    # Açma mantığı
│   └── utils.c     # Yardımcı fonksiyonlar
├── include/    # Header dosyaları
│   ├── tarsau.h    # Ortak sabitler ve yapılar
│   ├── parser.h    # Parser prototipleri
│   ├── pack.h      # Pack prototipleri
│   ├── unpack.h    # Unpack prototipleri
│   └── utils.h     # Yardımcı fonksiyon prototipleri
├── Makefile    # Derleme kuralları
└── README.md
```

## Temizlik

```bash
make clean
```

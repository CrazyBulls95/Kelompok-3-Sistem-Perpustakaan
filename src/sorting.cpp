
#include "sorting.h"
#include "validasi.h"

#include <algorithm>

// SORTIR BUKU
void sortirBuku() {
    if (daftarBuku.empty()) {
        cout << "\nBelum ada data buku!\n";
        return;
    }

    int pilihan;

    cout << "\n=== MENU SORTIR BUKU ===\n";
    cout << "1. Berdasarkan Jenis Buku\n";
    cout << "2. Judul A-Z\n";
    cout << "3. Tanggal Terbit Terlama\n";
    cout << "4. Tanggal Terbit Terbaru\n";

    pilihan = inputAngka("Pilih metode sortir: ");

    switch (pilihan) {

        case 1:
            sort(daftarBuku.begin(), daftarBuku.end(),
                [](Buku a, Buku b) {
                    if (a.jenis == b.jenis) {
                        return a.judul < b.judul;
                    }
                    return a.jenis < b.jenis;
                });
            break;

        case 2:
            sort(daftarBuku.begin(), daftarBuku.end(),
                [](Buku a, Buku b) {
                    return a.judul < b.judul;
                });
            break;

        case 3:
            sort(daftarBuku.begin(), daftarBuku.end(),
                [](Buku a, Buku b) {
                    if (a.tahun != b.tahun)
                        return a.tahun < b.tahun;

                    if (a.bulan != b.bulan)
                        return a.bulan < b.bulan;

                    return a.tanggal < b.tanggal;
                });
            break;

        case 4:
            sort(daftarBuku.begin(), daftarBuku.end(),
                [](Buku a, Buku b) {
                    if (a.tahun != b.tahun)
                        return a.tahun > b.tahun;

                    if (a.bulan != b.bulan)
                        return a.bulan > b.bulan;

                    return a.tanggal > b.tanggal;
                });
            break;

        default:
            cout << "\nPilihan sortir tidak valid!\n";
            return;
    }

    cout << "\nData berhasil diurutkan!\n";
    tampilkanTabel(daftarBuku);
}

// FILTER BUKU
void filterBuku() {
    if (daftarBuku.empty()) {
        cout << "\nBelum ada data buku!\n";
        return;
    }

    cout << "\n=== FILTER BUKU ===\n";

    string kategori = inputJenis();

    vector<Buku> hasilFilter;

    for (const Buku &b : daftarBuku) {
        if (b.jenis == kategori) {
            hasilFilter.push_back(b);
        }
    }

    if (hasilFilter.empty()) {
        cout << "\nTidak ada buku dalam kategori tersebut!\n";
    } else {
        cout << "\nHasil filter kategori " << kategori << ":\n";
        tampilkanTabel(hasilFilter);
    }
}
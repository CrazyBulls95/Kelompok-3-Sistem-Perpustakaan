
#include "validasi.h"

#include <iomanip>
#include <sstream>

// VALIDASI INPUT ANGKA
int inputAngka(string pesan) {
    string input;
    int angka;

    while (true) {
        cout << pesan;
        getline(cin, input);

        try {
            size_t posisi;
            angka = stoi(input, &posisi);

            if (posisi == input.length()) {
                return angka;
            }
        }
        catch (...) {
        }

        cout << "Input harus berupa angka!\n";
    }
}

// VALIDASI TANGGAL
bool tanggalValid(int t, int b, int y) {
    if (y < 1 || b < 1 || b > 12 || t < 1) {
        return false;
    }

    int jumlahHari[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    bool kabisat =
        (y % 400 == 0) ||
        (y % 4 == 0 && y % 100 != 0);

    if (kabisat) {
        jumlahHari[1] = 29;
    }

    return t <= jumlahHari[b - 1];
}

// INPUT JENIS BUKU
string inputJenis() {
    int pilihan;

    do {
        cout << "\nPilih Jenis Buku:\n";
        cout << "1. Fiksi\n";
        cout << "2. Nonfiksi\n";
        cout << "3. Pendidikan\n";
        cout << "4. Teknologi\n";
        cout << "5. Sejarah\n";
        cout << "6. Agama\n";

        pilihan = inputAngka("Masukkan pilihan: ");

        switch (pilihan) {
            case 1: return "Fiksi";
            case 2: return "Nonfiksi";
            case 3: return "Pendidikan";
            case 4: return "Teknologi";
            case 5: return "Sejarah";
            case 6: return "Agama";
            default:
                cout << "Pilihan tidak valid!\n";
        }

    } while (true);
}

// INPUT TANGGAL BUKU
void inputTanggal(Buku &b) {
    do {
        cout << "\nTanggal terbit buku\n";

        b.tanggal = inputAngka("Tanggal: ");
        b.bulan = inputAngka("Bulan: ");
        b.tahun = inputAngka("Tahun: ");

        if (!tanggalValid(b.tanggal, b.bulan, b.tahun)) {
            cout << "Tanggal tidak valid! Silakan ulangi.\n";
        }

    } while (!tanggalValid(b.tanggal, b.bulan, b.tahun));
}

// MENAMPILKAN DATA DALAM TABEL
void tampilkanTabel(const vector<Buku> &data) {
    if (data.empty()) {
        cout << "\nData buku masih kosong!\n";
        return;
    }

    cout << "\n";
    cout << left
         << setw(5) << "ID"
         << setw(25) << "Judul Buku"
         << setw(15) << "Jenis"
         << setw(20) << "Penulis"
         << "Tanggal Terbit\n";

    cout << string(85, '-') << "\n";

    for (const Buku &b : data) {
        ostringstream tanggal;

        tanggal << setfill('0')
                << setw(2) << b.tanggal << "-"
                << setw(2) << b.bulan << "-"
                << setw(4) << b.tahun;

        cout << left
             << setw(5) << b.id
             << setw(25) << b.judul
             << setw(15) << b.jenis
             << setw(20) << b.penulis
             << tanggal.str() << "\n";
    }
}


#include "buku.h"
#include "validasi.h"

vector<Buku> daftarBuku;
int nextID = 1;

// MENAMBAH BUKU
void tambahBuku() {
    Buku b;

    cout << "\n=== TAMBAH DATA BUKU ===\n";

    b.id = nextID++;

    cout << "Masukkan judul buku: ";
    getline(cin, b.judul);

    while (b.judul.empty()) {
        cout << "Judul tidak boleh kosong: ";
        getline(cin, b.judul);
    }

    b.jenis = inputJenis();

    cout << "Masukkan nama penulis: ";
    getline(cin, b.penulis);

    while (b.penulis.empty()) {
        cout << "Penulis tidak boleh kosong: ";
        getline(cin, b.penulis);
    }

    inputTanggal(b);

    daftarBuku.push_back(b);

    cout << "\nBuku berhasil ditambahkan!\n";
}

// MENGHAPUS BUKU
void hapusBuku() {
    cout << "\n=== HAPUS DATA BUKU ===\n";

    tampilkanTabel(daftarBuku);

    if (daftarBuku.empty()) {
        return;
    }

    int id = inputAngka("Masukkan ID buku yang ingin dihapus: ");

    for (int i = 0; i < daftarBuku.size(); i++) {
        if (daftarBuku[i].id == id) {
            daftarBuku.erase(daftarBuku.begin() + i);

            cout << "\nBuku berhasil dihapus!\n";
            return;
        }
    }

    cout << "\nID buku tidak ditemukan!\n";
}

// MENGEDIT BUKU
void editBuku() {
    cout << "\n=== EDIT DATA BUKU ===\n";

    tampilkanTabel(daftarBuku);

    if (daftarBuku.empty()) {
        return;
    }

    int id = inputAngka("Masukkan ID buku yang ingin diedit: ");

    for (Buku &b : daftarBuku) {
        if (b.id == id) {
            cout << "\nMasukkan data baru\n";

            cout << "Judul buku: ";
            getline(cin, b.judul);

            while (b.judul.empty()) {
                cout << "Judul tidak boleh kosong: ";
                getline(cin, b.judul);
            }

            b.jenis = inputJenis();

            cout << "Nama penulis: ";
            getline(cin, b.penulis);

            while (b.penulis.empty()) {
                cout << "Penulis tidak boleh kosong: ";
                getline(cin, b.penulis);
            }

            inputTanggal(b);

            cout << "\nData buku berhasil diperbarui!\n";
            return;
        }
    }

    cout << "\nID buku tidak ditemukan!\n";
}

// MENAMPILKAN SEMUA BUKU
void tampilkanSemuaBuku() {
    cout << "\n=== DAFTAR SEMUA BUKU ===\n";

    tampilkanTabel(daftarBuku);
}
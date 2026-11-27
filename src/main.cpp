
#include <iostream>

#include "buku.h"
#include "sorting.h"
#include "validasi.h"

using namespace std;

int main() {

    int pilihan;

    do {
        cout << "\n";
        cout << "========================================\n";
        cout << "    SISTEM MANAJEMEN BUKU PERPUSTAKAAN\n";
        cout << "========================================\n";

        cout << "1. Tambah Buku\n";
        cout << "2. Hapus Buku\n";
        cout << "3. Edit Data Buku\n";
        cout << "4. Sortir Buku\n";
        cout << "5. Filter Buku\n";
        cout << "6. Tampilkan Semua Buku\n";
        cout << "7. Keluar\n";

        cout << "========================================\n";

        pilihan = inputAngka("Masukkan pilihan: ");

        switch (pilihan) {
            case 1:
                tambahBuku();
                break;

            case 2:
                hapusBuku();
                break;

            case 3:
                editBuku();
                break;

            case 4:
                sortirBuku();
                break;

            case 5:
                filterBuku();
                break;

            case 6:
                tampilkanSemuaBuku();
                break;

            case 7:
                cout << "\nTerima kasih telah menggunakan program!\n";
                break;

            default:
                cout << "\nPilihan tidak valid!\n";
        }

    } while (pilihan != 7);

    return 0;
}

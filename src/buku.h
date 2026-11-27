
#ifndef BUKU_H
#define BUKU_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Buku {
    int id;
    string judul;
    string jenis;
    string penulis;
    int tanggal;
    int bulan;
    int tahun;
};

extern vector<Buku> daftarBuku;
extern int nextID;

void tambahBuku();
void hapusBuku();
void editBuku();
void tampilkanSemuaBuku();

#endif
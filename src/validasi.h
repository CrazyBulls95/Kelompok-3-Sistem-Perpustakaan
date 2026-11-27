
#ifndef VALIDASI_H
#define VALIDASI_H

#include "buku.h"

int inputAngka(string pesan);
bool tanggalValid(int t, int b, int y);
string inputJenis();
void inputTanggal(Buku &b);
void tampilkanTabel(const vector<Buku> &data);

#endif

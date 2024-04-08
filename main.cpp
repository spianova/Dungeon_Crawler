#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <random>
#include <fstream>
#include <string>
#include <array>

class Istota;
class Przedmiot;
class Gracz;
class Potwor;
class Gwiazdka;



class Mapa {
private:
    struct Pole {
        Gracz* gracz;
        Potwor* potwor;
        Przedmiot* przedmiot;
        Gwiazdka* gwiazdka;
        bool jestTunelem;
        int krokiDoZnikniecia;
        Pole() : gracz(nullptr), potwor(nullptr), przedmiot(nullptr),gwiazdka(nullptr), jestTunelem(false), krokiDoZnikniecia(-1) {}
    };

    std::vector<std::vector<Pole>> siatka;
    int szerokosc, wysokosc;
    std::mt19937 gen;
    std::uniform_int_distribution<> dist;

    void dodajSasiadowDoZbioru(int x, int y, std::set<std::pair<int, int>>& sciany) {
        if (x > 0) sciany.insert({x - 1, y});
        if (y > 0) sciany.insert({x, y - 1});
        if (x < szerokosc - 1) sciany.insert({x + 1, y});
        if (y < wysokosc - 1) sciany.insert({x, y + 1});
    }

public:
    Mapa(int szer, int wys) : szerokosc(szer), wysokosc(wys), gen(std::random_device{}()) {
        siatka.resize(wysokosc, std::vector<Pole>(szerokosc));
        generujLabirynt();
    }
    void generujLabirynt() {
        // Ustaw wszystkie pola jako ściany
        for (auto& wiersz : siatka)
            for (auto& pole : wiersz)
                pole.jestTunelem = false;

        // Rozpoczynamy od losowego punktu
        int x = gen() % szerokosc;
        int y = gen() % wysokosc;
        siatka[y][x].jestTunelem = true;

        std::set<std::pair<int, int>> sciany;
        dodajSasiadowDoZbioru(x, y, sciany);

        while (!sciany.empty()) {
            // Wybierz losową ścianę
            auto it = sciany.begin();
            std::advance(it, gen() % sciany.size());
            auto [wx, wy] = *it;
            sciany.erase(it);

            // Sprawdź, czy możemy przekształcić ścianę w tunel
            int otwarteSasiedztwo = 0;
            if (wx > 0) otwarteSasiedztwo += siatka[wy][wx - 1].jestTunelem;
            if (wy > 0) otwarteSasiedztwo += siatka[wy - 1][wx].jestTunelem;
            if (wx < szerokosc - 1) otwarteSasiedztwo += siatka[wy][wx + 1].jestTunelem;
            if (wy < wysokosc - 1) otwarteSasiedztwo += siatka[wy + 1][wx].jestTunelem;

            if (otwarteSasiedztwo == 1) {
                siatka[wy][wx].jestTunelem = true;
                dodajSasiadowDoZbioru(wx, wy, sciany);
            }
        }
    }

    bool czyMoznaPrzejsc(int x, int y) const {
        return x >= 0 && x < szerokosc && y >= 0 && y < wysokosc && siatka[y][x].jestTunelem;
    }

    int getSzer() const {
        return szerokosc;
    }
    int getWys() const {
        return wysokosc;
    }

    bool czyMoznaUmiescicIstote(int x, int y) {
        return x >= 0 && x < szerokosc && y >= 0 && y < wysokosc && siatka[y][x].gracz == nullptr && siatka[y][x].potwor == nullptr;
    }

    bool czyMoznaUmiescicPrzedmiot(int x, int y) {
        return x >= 0 && x < szerokosc && y >= 0 && y < wysokosc && siatka[y][x].przedmiot == nullptr;
    }

    bool czyPoleZajete(int x, int y) const{
        return siatka[y][x].gracz != nullptr || siatka[y][x].potwor != nullptr;
    }

    //czy mozna umiescic istote zeby ona miala gdzie isc
    bool czyMaDostepnyRuch(int x, int y) {

        const std::array<std::pair<int, int>, 4> kierunki = {{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};
        for (const auto& k : kierunki) {
            int nx = x + k.first, ny = y + k.second;
            if (nx >= 0 && nx < szerokosc && ny >= 0 && ny < wysokosc && siatka[ny][nx].jestTunelem) {
                return true; // Znaleziono dostępny ruch
            }
        }
        return false; // Brak dostępnych ruchów
    }

    void umiescGracza(Gracz* gracz, int x, int y) {
        if (czyMoznaUmiescicIstote(x, y)) {
            siatka[y][x].gracz = gracz;
        }
    }

    void umiescPotwora(Potwor* potwor, int x, int y) {
        if (czyMoznaUmiescicIstote(x, y) && czyMaDostepnyRuch(x, y)) {
            siatka[y][x].potwor = potwor;
        }
    }

    void umiescPrzedmiot(Przedmiot* przedmiot, int x, int y) {
        if (czyMoznaUmiescicPrzedmiot(x, y)) {
            siatka[y][x].przedmiot = przedmiot;
        }
    }

    void usunGracza(int x, int y) {
        if (x >= 0 && x < szerokosc && y >= 0 && y < wysokosc) {
            siatka[y][x].gracz = nullptr;
        }
    }

    void usunPotwora(int x, int y) {
        if (x >= 0 && x < szerokosc && y >= 0 && y < wysokosc) {
            siatka[y][x].potwor = nullptr;
        }
    }

    void usunPrzedmiot(int x, int y) {
        if (x >= 0 && x < szerokosc && y >= 0 && y < wysokosc) {
            siatka[y][x].przedmiot = nullptr;
        }
    }
    void wyswietlMape( Gracz* gracz);

    bool czyJestPotworTypu(int x, int y, const std::string& typ);
    void rozmiescGwiazdki(int liczbaGwiazdek);

    void aktualizujPozycjePotwora(Potwor* potwor, int staryX, int staryY, int nowyX, int nowyY) {
        if (staryX >= 0 && staryX < szerokosc && staryY >= 0 && staryY < wysokosc) {
            siatka[staryY][staryX].potwor = nullptr; // Usuń potwora ze starej pozycji
        }
        if (nowyX >= 0 && nowyX < szerokosc && nowyY >= 0 && nowyY < wysokosc && siatka[nowyY][nowyX].jestTunelem) {
            siatka[nowyY][nowyX].potwor = potwor; // Umieść potwora na nowej pozycji
        }
    }

};


// Klasa Istota

class Istota {
protected:
    int zdrowie;
    int sila;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dist;

public:
    Istota(int zdrowie, int sila) : zdrowie(zdrowie), sila(sila), gen(rd()), dist(1, 6) {}

    void lecz(int ilosc) {
        zdrowie += ilosc;
        if (zdrowie > 100) zdrowie = 100;
    }

    void ran(int ilosc) {
        zdrowie -= ilosc;
        if (zdrowie < 0) zdrowie = 0;
    }

    bool czyZyje() {
        return zdrowie > 0;
    }

    void atakuj(Istota& cel) {
        int obrazenia = sila + dist(gen); // Dodanie losowości do siły ataku
        cel.ran(obrazenia);
        std::cout << "Zadano " << obrazenia << " punktow obrazen." << std::endl;
    }

    // Metody do uzyskiwania informacji o stanie istoty
    int getZdrowie() const {
        return zdrowie;
    }
};

class Przedmiot {
public:
    virtual void uzyj(Gracz& gracz) = 0;
};


class Gracz : public Istota {
private:
    int inteligencja;
    int zrecznosc;
    int szczescie;
    std::vector<Przedmiot*> ekwipunek;
    int energia;
    int poziomGlodu;
    int poziomPragnienia;
    int krokiOczekiwania;

public:
    Gracz(int zdrowie, int sila, int intel, int zrecz, int szcz, int energia)
            : Istota(zdrowie, sila), inteligencja(intel), zrecznosc(zrecz), szczescie(szcz) , energia(energia), poziomGlodu(0), poziomPragnienia(0), krokiOczekiwania(0){}

    int getSila() const { return sila; }
    int getInteligencja() const { return inteligencja; }
    int getZrecznosc() const { return zrecznosc; }
    int getSzczescie() const { return szczescie; }
    int getEnergia() const { return energia; }
    int getZdrowie() const {
        return zdrowie;
    }

    void dodajDoEkwipunku(Przedmiot* przedmiot) {
        ekwipunek.push_back(przedmiot);
    }

    void uzyjPrzedmiotu(int indeks) {
        if (indeks >= 0 && indeks < ekwipunek.size()) {
            ekwipunek[indeks]->uzyj(*this);
        }
    }
    void usunPrzedmiot(int indeks) {
        if (indeks >= 0 && indeks < ekwipunek.size()) {
            ekwipunek.erase(ekwipunek.begin() + indeks);
        }
    }

    // Metody dla umiejętności

    void szybkiUnik() {
        if (zrecznosc > 10) {
        }
    }

    void kulaOgnia(Potwor& cel) {
        if (inteligencja > 10) {
        }
    }

    void szczesliweZnalezisko() {
        if (szczescie > 10) {
        }
    }

    void mocneUderzenie(Potwor& cel);

    void zwiekszSile(int bonus);

    void zmniejszEnergie(int ilosc) {
        energia -= ilosc;
        if (energia < 0) energia = 0;
    }

    void odnowEnergie() {
        if (energia == 0) {
            krokiOczekiwania++;
            if (krokiOczekiwania >= 2) {
                energia = 25;
                krokiOczekiwania = 0;
            }
        }
    }
    void lecz(int ilosc)  {
        zdrowie += ilosc;
    }

    int getKrokiOczekiwania() const { return krokiOczekiwania; }
};


class Gwiazdka : public Przedmiot {
public:
    void uzyj(Gracz& gracz) override {
        gracz.lecz(5); // Zwiększa zdrowie gracza o 5
        std::cout << "Zdrowie zwiekszone o 5 punktow." << std::endl;
    }
};
void Mapa::rozmiescGwiazdki(int liczbaGwiazdek) {
    while (liczbaGwiazdek > 0) {
        int x = gen() % szerokosc;
        int y = gen() % wysokosc;
        if (siatka[y][x].jestTunelem && czyMoznaUmiescicPrzedmiot(x, y) && siatka[y][x].gwiazdka == nullptr) {
            siatka[y][x].gwiazdka = new Gwiazdka();
            siatka[y][x].krokiDoZnikniecia = 6;
            liczbaGwiazdek--;
            umiescPrzedmiot(siatka[y][x].gwiazdka, x, y);
        }
    }
}

class Potwor : public Istota {
    int x, y;
public:
    std::string typ;
    Potwor(int zdrowie, int sila, std::string typ) : Istota(zdrowie, sila), typ(typ) {}
    void atakaPotwora(Istota& cel, Mapa& mapa, int x, int y, bool& graTrwa) {
        int bonus = 0;
        //  sąsiednie pola na obecność innych potworów tego samego typu
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (mapa.czyJestPotworTypu(x + dx, y + dy, typ)) {
                    bonus++;
                }
            }
        }
        int obrazenia = sila + bonus; // Dodaj bonus za każdego sąsiedniego potwora
        cel.ran(obrazenia);
        std::cout << "Zadano " << obrazenia << " punktow obrazen." << std::endl;
        if (!cel.czyZyje()) {
            std::cout << "Potwor " << typ << " zabija cie." << std::endl;
            graTrwa = false; // Kończy grę
        }
    }

};

class SlabyPotwor : public Potwor {
public:
    SlabyPotwor() : Potwor(20, 3, "Slaby") {} // Najsłabszy potwór
};

class KolejnyPotwor : public Potwor {
public:
    KolejnyPotwor() : Potwor(30, 5, "Kolejny") {} // Średnio silny potwór
};

class RownyPotwor : public Potwor {
public:
    RownyPotwor() : Potwor(40, 7, "Rowny") {} // Równy potwór
};

class OgromneZagrozeniePotwor : public Potwor {
public:
    OgromneZagrozeniePotwor() : Potwor(50, 9, "OgromneZagrozenie") {} // Ogromne zagrożenie
};

class OstatniPotwor : public Potwor {
public:
    OstatniPotwor() : Potwor(60, 11, "Ostatni") {} // Ostatni potwór
};
void Mapa::wyswietlMape( Gracz* gracz) {
    for (int y = 0; y < wysokosc; ++y) {
        for (int x = 0; x < szerokosc; ++x) {
            if (siatka[y][x].gracz == gracz){
                if (siatka[y][x].gwiazdka != nullptr){
                    if(gracz) {
                        gracz->lecz(5); // Zwiększa zdrowie gracza o 5
                    }
                    delete siatka[y][x].gwiazdka;
                    siatka[y][x].gwiazdka = nullptr;
                }
                std::cout << 'G';
            }

            else if (siatka[y][x].potwor != nullptr) {
                if (siatka[y][x].potwor->typ == "Slaby") std::cout << 'S';
                else if (siatka[y][x].potwor->typ == "Kolejny") std::cout << 'K';
                else if (siatka[y][x].potwor->typ == "Rowny") std::cout << 'R';
                else if (siatka[y][x].potwor->typ == "OgromneZagrozenie") std::cout << 'Z';
                else if (siatka[y][x].potwor->typ == "Ostatni") std::cout << 'O';
                else std::cout << 'I'; // Domyślnie, jeśli nie pasuje do żadnego typu
            } else if (siatka[y][x].gwiazdka != nullptr) {
                if (siatka[y][x].krokiDoZnikniecia > 0) {
                    std::cout << '*';
                    siatka[y][x].krokiDoZnikniecia--;
                } else {
                    delete siatka[y][x].gwiazdka;
                    siatka[y][x].gwiazdka = nullptr;
                    std::cout << ' ';  // Gwiazdka zniknęła
                }
            }
            else if (siatka[y][x].jestTunelem)
                std::cout << ' ';  // Pusta przestrzeń w tunelu
            else
                std::cout << '#';  // Ściana tunelu

        }
        std::cout << std::endl;
    }
}

void losowyRuchPotwora(Potwor*& potwor, int& x, int& y, Mapa& mapa) {
    if (potwor && !potwor->czyZyje()) {
        return; // Nie wykonuj ruchu, jeśli potwór nie żyje
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 3);
    int poprzedniX = x, poprzedniY = y;
    int nowyX = x, nowyY = y;

    int kierunek = dis(gen); // Losowanie kierunku: 0-góra, 1-dół, 2-lewo, 3-prawo

    switch (kierunek) {
        case 0: nowyY = y - 1; break; // Góra
        case 1: nowyY = y + 1; break; // Dół
        case 2: nowyX = x - 1; break; // Lewo
        case 3: nowyX = x + 1; break; // Prawo
    }
    if (mapa.czyMoznaPrzejsc(nowyX, nowyY) && !mapa.czyPoleZajete(nowyX, nowyY)) {
        mapa.usunPotwora(poprzedniX, poprzedniY); // Usuń potwora ze starej pozycji
        x = nowyX;
        y = nowyY;
        mapa.umiescPotwora(potwor, x, y); // Umieść potwora na nowej pozycji
    }
}

void wczytajPotwory(std::vector<Potwor*>& potwory) {
    std::ifstream plik("potwory.txt");
    std::string typ;
    while (plik >> typ) {
        if (typ == "Slaby") potwory.push_back(new SlabyPotwor());
        else if (typ == "Kolejny") potwory.push_back(new KolejnyPotwor());
        else if (typ == "Rowny") potwory.push_back(new RownyPotwor());
        else if (typ == "OgromneZagrozenie") potwory.push_back(new OgromneZagrozeniePotwor());
        else if (typ == "Ostatni") potwory.push_back(new OstatniPotwor());

    }
}

bool Mapa::czyJestPotworTypu(int x, int y, const std::string& typ) {
    if (x >= 0 && x < szerokosc && y >= 0 && y < wysokosc) {
        Potwor* p = siatka[y][x].potwor;
        return p != nullptr && p->typ == typ;
    }
    return false;
}

Potwor* stworzPotwora(const std::string& typ) {
    if (typ == "Slaby") {
        return new SlabyPotwor();
    } else if (typ == "Kolejny") {
        return new KolejnyPotwor();
    } else if (typ == "Rowny") {
        return new RownyPotwor();
    } else if (typ == "OgromneZagrozenie") {
        return new OgromneZagrozeniePotwor();
    } else if (typ == "Ostatni") {
        return new OstatniPotwor();
    }
    return nullptr;
}
//dla gracza mocne uderzenie
void Gracz::mocneUderzenie(Potwor& cel) {
    const int kosztEnergii = 5;  // Koszt energii dla wykonania mocnego uderzenia
    const int dodatkoweObrazenia = 5;  // Stała wartość dodatkowych obrażeń

    if (energia >= kosztEnergii) {
        int obrazenia = sila + dodatkoweObrazenia;  // Podstawowe obrażenia plus dodatkowe
        cel.ran(obrazenia);  // Zaaplikuj obrażenia potworowi

        energia -= kosztEnergii;  // Zmniejsz energię gracza
        std::cout << "Wykonano mocne uderzenie, zadano " << obrazenia << " punktów obrażeń." << std::endl;
    } else {
        std::cout << "Za malo energii na wykonanie mocnego uderzenia!" << std::endl;
    }
}
void Gracz::zwiekszSile(int bonus) {
    sila += bonus;  // Zwiększanie siły gracza
    std::cout << "Sila zwiekszona o " << bonus << "." << std::endl;
}


class Bron : public Przedmiot {
    int bonusDoObrazen;

public:
    Bron(int bonus) : bonusDoObrazen(bonus) {}

    void uzyj(Gracz& gracz) override {
        gracz.zwiekszSile(bonusDoObrazen);
        std::cout << "Uzyto broni. Sila zwiekszona o " << bonusDoObrazen << "." << std::endl;
    }
};

class Zbroja : public Przedmiot {
    int bonusDoObrony;

public:
    Zbroja(int bonus) : bonusDoObrony(bonus) {}

    void uzyj(Gracz& gracz) override {
        // Logika użycia zbroi
    }
};

// klasa Przedmiot

int main() {
    Mapa mapa(50, 10);
    Gracz gracz(100, 10, 5, 5, 5, 100); // Zdrowie 100, Siła 10
    Potwor* potwor = nullptr;
    Potwor* potwor2 = nullptr;
    Potwor* potwor3 = nullptr;
    Potwor* potwor4 = nullptr;
    Potwor* potwor5 = nullptr;
    Potwor* potwor6 = nullptr;
    Bron* miecz = new Bron(5);

    std::cout << "Poczatkowe statystyki gracza:" << std::endl;
    std::cout << "Zdrowie: " << gracz.getZdrowie() << std::endl;
    std::cout << "Sila: " << gracz.getSila() << std::endl;
    std::cout << "Inteligencja: " << gracz.getInteligencja() << std::endl;
    std::cout << "Zrecznosc: " << gracz.getZrecznosc() << std::endl;
    std::cout << "Szczescie: " << gracz.getSzczescie() << std::endl;
    std::cout << "Energia: " << gracz.getEnergia() << std::endl;

    //  gracz i potwor na mapie
    int graczX = 0, graczY = 0;
    int potworX = 3, potworY = 3;
    int potwor2X = 0, potwor2Y = 9;
    int potwor3X = 6, potwor3Y = 0;
    int potwor4X = 20, potwor4Y = 8;
    int potwor5X = 18, potwor5Y = 1;
    int potwor6X = 5, potwor6Y = 9;

    std::vector<std::string> potworTypy = {"Slaby", "Kolejny", "Rowny", "OgromneZagrozenie", "Ostatni"};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, potworTypy.size() - 1);

    // Randomly select a monster type
    std::string selectedPotworTyp = potworTypy[dis(gen)];
    std::string selectedPotworTyp2 = potworTypy[dis(gen)];
    std::string selectedPotworTyp3 = potworTypy[dis(gen)];
    std::string selectedPotworTyp4 = potworTypy[dis(gen)];
    std::string selectedPotworTyp5 = potworTypy[dis(gen)];
    std::string selectedPotworTyp6 = potworTypy[dis(gen)];

    potwor = stworzPotwora(selectedPotworTyp);
    potwor2 = stworzPotwora(selectedPotworTyp2);
    potwor3 = stworzPotwora(selectedPotworTyp3);
    potwor4 = stworzPotwora(selectedPotworTyp4);
    potwor5 = stworzPotwora(selectedPotworTyp5);
    potwor6 = stworzPotwora(selectedPotworTyp6);

    mapa.umiescGracza(&gracz, graczX, graczY);
    mapa.rozmiescGwiazdki(10);
    if(potwor && potwor->czyZyje()) {
        mapa.umiescPotwora(potwor, potworX, potworY);
    }
    if(potwor2 && potwor2->czyZyje()){
        mapa.umiescPotwora(potwor2, potwor2X, potwor2Y);
    }
    if(potwor3 && potwor3->czyZyje()) {
        mapa.umiescPotwora(potwor3, potwor3X, potwor3Y);
    }
    if(potwor4 && potwor4->czyZyje()){
        mapa.umiescPotwora(potwor4, potwor4X, potwor4Y);
    }
    if(potwor5 && potwor5->czyZyje()) {
        mapa.umiescPotwora(potwor5, potwor5X, potwor5Y);
    }
    if(potwor6 && potwor6->czyZyje()){
        mapa.umiescPotwora(potwor6, potwor6X, potwor6Y);
    }

    gracz.dodajDoEkwipunku(miecz); // Dodanie broni do ekwipunku gracza
    gracz.uzyjPrzedmiotu(0); // Gracz używa broni

    std::random_device randomDevice;
    std::mt19937 gene(randomDevice());
    std::uniform_int_distribution<> dist(0, 5); // Zakładając, że w każdym cyklu może pojawić się od 0 do 5 gwiazdek

    char ruch;
    bool graTrwa = true;

    while (graTrwa && gracz.czyZyje() && (potwor&&potwor->czyZyje() || potwor2&&potwor2->czyZyje() || potwor3&&potwor3->czyZyje() || potwor4&&potwor4->czyZyje() || potwor5&&potwor5->czyZyje() || potwor6&&potwor6->czyZyje())){
        int liczbaGwiazdekDoDodania = dist(gene);
        mapa.rozmiescGwiazdki(liczbaGwiazdekDoDodania);

        mapa.wyswietlMape(&gracz);

        std::cout << "Wybierz ruch (w/a/s/d), atak (f) lub wyjscie (q): ";
        std::cin >> ruch;
        bool wykonanoAkcje = false;

        mapa.usunGracza(graczX,graczY);
//gracz
        switch (ruch) {
            case 'w': // góra
                if (mapa.czyMoznaUmiescicIstote(graczX, graczY - 1) && mapa.czyMoznaPrzejsc(graczX, graczY - 1)) {
                    mapa.usunGracza(graczX, graczY);
                    graczY--;
                    mapa.umiescGracza(&gracz, graczX, graczY);
                }
                gracz.zmniejszEnergie(1);
                wykonanoAkcje = true;
                break;
            case 's': // dół
                if (mapa.czyMoznaUmiescicIstote(graczX, graczY + 1) && mapa.czyMoznaPrzejsc(graczX, graczY + 1)) {
                    mapa.usunGracza(graczX, graczY);
                    graczY++;
                    mapa.umiescGracza(&gracz, graczX, graczY);
                }
                gracz.zmniejszEnergie(1);
                wykonanoAkcje = true;
                break;
            case 'a': // lewo
                if (mapa.czyMoznaUmiescicIstote(graczX - 1, graczY)&& mapa.czyMoznaPrzejsc(graczX - 1, graczY )) {
                    mapa.usunGracza(graczX, graczY);
                    graczX--;
                    mapa.umiescGracza(&gracz, graczX, graczY);
                }
                gracz.zmniejszEnergie(1);
                wykonanoAkcje = true;
                break;
            case 'd': // prawo
                if (mapa.czyMoznaUmiescicIstote(graczX + 1, graczY) && mapa.czyMoznaPrzejsc(graczX + 1, graczY )) {
                    mapa.usunGracza(graczX, graczY);
                    graczX++;
                    mapa.umiescGracza(&gracz, graczX, graczY);
                }
                gracz.zmniejszEnergie(1);
                wykonanoAkcje = true;
                break;
            case 'f': // atak
                if (potwor && potwor->czyZyje() && graczX == potworX && abs(graczY - potworY) <= 1 ||
                    graczY == potworY && abs(graczX - potworX) <= 1 ) {
                    gracz.atakuj(*potwor);
                    if (!potwor->czyZyje()) {
                        mapa.usunPotwora(potworX, potworY);
                        delete potwor;
                        potwor = nullptr; // Ustaw wskaźnik na nullptr po usunięciu obiektu
                    }
                    gracz.zmniejszEnergie(5);
                    wykonanoAkcje = true;
                } else if (potwor2 && potwor2->czyZyje() && graczX == potwor2X && abs(graczY - potwor2Y) <= 1 ||
                           graczY == potwor2Y && abs(graczX - potwor2X) <= 1) {
                    gracz.atakuj(*potwor2);
                    if (!potwor2->czyZyje()) {
                        mapa.usunPotwora(potwor2X, potwor2Y);
                        delete potwor2;
                        potwor2 = nullptr; // Ustaw wskaźnik na nullptr po usunięciu obiektu
                    }
                    gracz.zmniejszEnergie(5);
                    wykonanoAkcje = true;
                }else if (potwor3 && potwor3->czyZyje() && graczX == potwor3X && abs(graczY - potwor3Y) <= 1 ||
                          graczY == potwor3Y && abs(graczX - potwor3X) <= 1) {
                    gracz.atakuj(*potwor3);
                    if (!potwor3->czyZyje()) {
                        mapa.usunPotwora(potwor3X, potwor3Y);
                        delete potwor3;
                        potwor3 = nullptr; // Ustaw wskaźnik na nullptr po usunięciu obiektu
                    }
                    gracz.zmniejszEnergie(5);
                    wykonanoAkcje = true;
                }else if (potwor4 && potwor4->czyZyje() && graczX == potwor4X && abs(graczY - potwor4Y) <= 1 ||
                          graczY == potwor4Y && abs(graczX - potwor4X) <= 1) {
                    gracz.atakuj(*potwor4);
                    if (!potwor4->czyZyje()) {
                        mapa.usunPotwora(potwor4X, potwor4Y);
                        delete potwor4;
                        potwor4 = nullptr; // Ustaw wskaźnik na nullptr po usunięciu obiektu
                    }
                    gracz.zmniejszEnergie(5);
                    wykonanoAkcje = true;
                }else if (potwor5 && potwor5->czyZyje() && graczX == potwor5X && abs(graczY - potwor5Y) <= 1 ||
                          graczY == potwor5Y && abs(graczX - potwor5X) <= 1) {
                    gracz.atakuj(*potwor5);
                    if (!potwor5->czyZyje()) {
                        mapa.usunPotwora(potwor5X, potwor5Y);
                        delete potwor5;
                        potwor5 = nullptr; // Ustaw wskaźnik na nullptr po usunięciu obiektu
                    }
                    gracz.zmniejszEnergie(5);
                    wykonanoAkcje = true;
                }else if (potwor6 && potwor6->czyZyje() && graczX == potwor6X && abs(graczY - potwor6Y) <= 1 ||
                          graczY == potwor6Y && abs(graczX - potwor6X) <= 1) {
                    gracz.atakuj(*potwor6);
                    if (!potwor6->czyZyje()) {
                        mapa.usunPotwora(potwor6X, potwor6Y);
                        delete potwor6;
                        potwor6 = nullptr; // Ustaw wskaźnik na nullptr po usunięciu obiektu
                    }
                    gracz.zmniejszEnergie(5);
                    wykonanoAkcje = true;
                }else{
                    std::cout << "Nie ma wroga w zasiegu!" << std::endl;
                }
                break;
            case 'q':
                graTrwa = false;
                break;
            default:
                std::cout << "Nieznana komenda!" << std::endl;
                break;

        }
        if (!wykonanoAkcje) {
            gracz.odnowEnergie();
        }
        mapa.umiescGracza(&gracz,graczX,graczY);


//potwór
        if (potwor && !potwor->czyZyje()) {
            mapa.usunPotwora(potworX, potworY);
            delete potwor;
            potwor = nullptr;
        }
        if (potwor2 && !potwor2->czyZyje()) {
            mapa.usunPotwora(potwor2X, potwor2Y);
            delete potwor2;
            potwor2 = nullptr;
        }
        if (potwor3 && !potwor3->czyZyje()) {
            mapa.usunPotwora(potwor3X, potwor3Y);
            delete potwor3;
            potwor3 = nullptr;
        }
        if (potwor4 && !potwor4->czyZyje()) {
            mapa.usunPotwora(potwor4X, potwor4Y);
            delete potwor4;
            potwor4 = nullptr;
        }
        if (potwor5 && !potwor5->czyZyje()) {
            mapa.usunPotwora(potwor5X, potwor5Y);
            delete potwor5;
            potwor5 = nullptr;
        }
        if (potwor6 && !potwor6->czyZyje()) {
            mapa.usunPotwora(potwor6X, potwor6Y);
            delete potwor6;
            potwor6 = nullptr;
        }

        if (potwor && potwor->czyZyje()) {
                losowyRuchPotwora(potwor,potworX, potworY, mapa);
                mapa.umiescPotwora(potwor,potworX,potworY);
            if(abs(graczX - potworX) + abs(graczY - potworY)<=1) {
                potwor->atakaPotwora(gracz,mapa,graczX, graczY, graTrwa);
            }
        }
        //sprawdzIZaktualizujPotwora(potwor, potworX, potworY);
        if (potwor2 && potwor2->czyZyje()) {
            losowyRuchPotwora(potwor2,potwor2X, potwor2Y, mapa);
            mapa.umiescPotwora(potwor2, potwor2X, potwor2Y);
            if (potwor2->czyZyje() && abs(graczX - potwor2X) + abs(graczY - potwor2Y) <= 1) {
                potwor2->atakaPotwora(gracz, mapa, graczX, graczY, graTrwa);
            }
        }

        //sprawdzIZaktualizujPotwora(potwor2, potwor2X, potwor2Y);
        if (potwor3 && potwor3->czyZyje()) {
            losowyRuchPotwora(potwor3,potwor3X, potwor3Y, mapa);
            mapa.umiescPotwora(potwor3, potwor3X, potwor3Y);
            if (potwor3->czyZyje() && abs(graczX - potwor3X) + abs(graczY - potwor3Y) <= 1) {
                potwor3->atakaPotwora(gracz, mapa, graczX, graczY, graTrwa);
            }
        }

        //sprawdzIZaktualizujPotwora(potwor3, potwor3X, potwor3Y);
        if (potwor4 && potwor4->czyZyje()) {
            losowyRuchPotwora(potwor4,potwor4X, potwor4Y, mapa);
            mapa.umiescPotwora(potwor4, potwor4X, potwor4Y);
            if (potwor4->czyZyje() && abs(graczX - potwor4X) + abs(graczY - potwor4Y) <= 1) {
                potwor4->atakaPotwora(gracz, mapa, graczX, graczY, graTrwa);
            }
        }

        //sprawdzIZaktualizujPotwora(potwor4, potwor4X, potwor4Y);
        if (potwor5 && potwor5->czyZyje()) {
            losowyRuchPotwora(potwor5,potwor5X, potwor5Y, mapa);
            mapa.umiescPotwora(potwor5, potwor5X, potwor5Y);
            if (potwor5->czyZyje() && abs(graczX - potwor5X) + abs(graczY - potwor5Y) <= 1) {
                potwor5->atakaPotwora(gracz, mapa, graczX, graczY, graTrwa);
            }
        }

        //sprawdzIZaktualizujPotwora(potwor5, potwor5X, potwor5Y);
        if (potwor6 && potwor6->czyZyje()) {
            losowyRuchPotwora(potwor6,potwor6X, potwor6Y, mapa);
            mapa.umiescPotwora(potwor6, potwor6X, potwor6Y);
            if (potwor6->czyZyje() && abs(graczX - potwor6X) + abs(graczY - potwor6Y) <= 1) {
                potwor6->atakaPotwora(gracz, mapa, graczX, graczY, graTrwa);
            }
        }

        //sprawdzIZaktualizujPotwora(potwor6, potwor6X, potwor6Y);

        std::cout << "Statystyki gracza:" << std::endl;
        std::cout << "Zdrowie: " << gracz.getZdrowie() << std::endl;
        std::cout << "Sila: " << gracz.getSila() << std::endl;
        std::cout << "Inteligencja: " << gracz.getInteligencja() << std::endl;
        std::cout << "Zrecznosc: " << gracz.getZrecznosc() << std::endl;
        std::cout << "Szczescie: " << gracz.getSzczescie() << std::endl;
        std::cout << "Energia: " << gracz.getEnergia() << std::endl;

        std::cout << " zdrowie potwora1: " << (potwor ? std::to_string(potwor->getZdrowie()) : "0")
                  << ", zdrowie potwora2: " << (potwor2 ? std::to_string(potwor2->getZdrowie()) : "0")
                  << ", zdrowie potwora3: " << (potwor3 ? std::to_string(potwor3->getZdrowie()) : "0")
                  << ", zdrowie potwora4: " << (potwor4 ? std::to_string(potwor4->getZdrowie()) : "0")
                  << ", zdrowie potwora5: " << (potwor5 ? std::to_string(potwor5->getZdrowie()) : "0")
                  << ", zdrowie potwora6: " << (potwor6 ? std::to_string(potwor6->getZdrowie()) : "0") << std::endl;
        // Sprawdzenie, czy wszystkie potwory zostały pokonane
        bool wszystkiePotworyPokonane = !potwor && !potwor2 && !potwor3 && !potwor4 && !potwor5 && !potwor6;
        if (wszystkiePotworyPokonane) {
            std::cout << "Gracz wygrywa!" << std::endl;
            graTrwa = false;
        }

    }
    delete potwor;
    delete potwor2;
    delete potwor3;
    delete potwor4;
    delete potwor5;
    delete potwor6;
    delete miecz;
    return 0;
}

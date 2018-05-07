#ifndef DSEXMON_H
#define DSEXMON_H

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <windows.h>
#include <tlhelp32.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Main_Window;
class Racing_Horse_Table;
class Process_Dialog;
class Process_Table;
template <class R>
class Sortable_Table;
struct farm_t;
struct racing_horse;
struct racing_horse_record;
class scope_exit;
struct stallion_t;

struct farm_t {
    char magic[4];
    char name[8];
    BYTE x_12_4[4];
    DWORD fund;
    BYTE x_20_4[4];
    BYTE date;
    BYTE x_25_7[7];
    char racing[35];
    char breeding[6];
    BYTE sales[4];
    BYTE x_77_3[3];
};

struct stallion_t {
    DWORD status;
    WORD pedigree[15];
    unsigned achievement() const;
    unsigned color() const;
    unsigned dart() const;
    unsigned distance_lower() const;
    unsigned distance_upper() const;
    unsigned fee() const;
    unsigned growth_type() const;
    unsigned guts() const;
    unsigned health() const;
    unsigned stability() const;
    unsigned temper() const;
};

struct racing_horse {
    BYTE x_0_2[2];
    BYTE name;
    BYTE age;
    BYTE sex_color;
    BYTE x_5_1[1];
    WORD mother;
    WORD father;
    BYTE x_10_14[14];
    int8_t weight;
    BYTE best_weight;
    BYTE max_speed;
    BYTE speed;
    BYTE max_stamina;
    BYTE stamina;
    BYTE max_guts;
    BYTE guts;
    BYTE max_temper;
    BYTE temper;
    BYTE x_34_1[1];
    BYTE growth;
    BYTE dart;
    BYTE health;
    BYTE x_38_2[2];
    BYTE condition;
    BYTE x_41_5[5];
    WORD prize;
    DWORD total_prize;
    struct {
        BYTE right_turf[4];
        BYTE right_dart[4];
        BYTE left_turf[4];
        BYTE left_dart[4];
    } result;
    BYTE x_68_6[6];
    unsigned color() const;
    unsigned growth_type() const;
    unsigned late_bloomer_bonus() const;
    int mare_complement(const stallion_t& fat) const;
    unsigned sex() const;
};

struct racing_horse_record {
    unsigned index;
    racing_horse horse;
    std::string name;
    stallion_t father;
};

template <class R>
class Sortable_Table : public Fl_Table_Row {
public:
    using compare_t = std::function<bool(R*,R*)>;
    std::vector<R> records;
    std::vector<R*> sorted_records;
    Sortable_Table(
        const int& x,
        const int& y,
        const int& wid,
        const int& hei,
        const compare_t* comps
    );
    void sort();
private:
    const compare_t* comps_;
    int col_, ord_;
    static void self_callback(Fl_Widget* wid, void* some);
};

class Main_Window : public Fl_Double_Window {
public:
    HANDLE process_handle;
    LPVOID dsex_header_address;
    farm_t farm;
    Main_Window();
    void load(const DWORD& procId);
    static void timer_callback(void* some);
private:
    std::shared_ptr<Fl_Button> procBut_;
    std::shared_ptr<Racing_Horse_Table> racHorTab_;
    std::shared_ptr<Process_Dialog> procDia_;
    std::shared_ptr<scope_exit> procHanExit_;
    static void procBut_callback(Fl_Widget* wid, void* some);
    static void racHorTab_callback(Fl_Widget* wid, void* some);
    void update();
};

class Racing_Horse_Table : public Sortable_Table<racing_horse_record> {
public:
    Racing_Horse_Table(
        const int& x,
        const int& y,
        const int& wid,
        const int& hei,
        Main_Window* mainWin
    );
protected:
    inline virtual void draw_cell(
        TableContext context,
        int R = 0,
        int C = 0,
        int X = 0,
        int Y = 0,
        int W = 0,
        int H = 0
    ) override;
private:
    Main_Window* mainWin_;
    static const Sortable_Table<racing_horse_record>::compare_t comps_[19];
};

class Process_Dialog : public Fl_Window {
public:
    Process_Dialog(Main_Window* mainWin);
    void load();
private:
    std::shared_ptr<Process_Table> procTab_;
    std::shared_ptr<Fl_Return_Button> okBut_;
    std::shared_ptr<Fl_Button> canBut_;
    Main_Window* mainWin_;
    static void okBut_callback(Fl_Widget* wid, void* some);
    static void canBut_callback(Fl_Widget* wid, void* some);
};

class Process_Table : public Sortable_Table<PROCESSENTRY32> {
public:
    Process_Table
        (const int& x, const int& y, const int& wid, const int& hei);
protected:
    inline virtual void draw_cell(
        TableContext context,
        int R = 0,
        int C = 0,
        int X = 0,
        int Y = 0,
        int W = 0,
        int H = 0
    ) override;
private:
    static const Sortable_Table<PROCESSENTRY32>::compare_t comps_[2];
};

class scope_exit {
public:
    scope_exit(const std::function<void()>& proc);
    ~scope_exit();
private:
    std::function<void()> proc_;
};

extern const std::string APP_NAME;
extern const std::string RHT_COL_HEADER_LABELS[19];
extern const int RHT_COL_WIDTH[19];
extern const Fl_Align RHT_COL_ALIGNS[19];
extern const std::string PROCESS_DIALOG_LABEL;
extern const std::string PT_COL_HEADER_LABELS[2];
extern const int PT_COL_WIDTH[2];
extern const Fl_Align PT_COL_ALIGNS[2];
extern const BYTE DSEX_HEADER[16];
extern const std::string NAME_LETTERS[128];
extern const std::string LOCATION_LABELS[3];
extern const std::string SEX_LABELS[3];
extern const std::string AGE_LABELS[9];
extern const std::string COLOR_LABELS[8];
extern const std::string GROWTH_TYPE_LABELS[5];

std::string Sprintf(const char* format, ...);
std::string ansi_to_utf8(const std::string& ansi);
std::string decode_horse_name(const std::string& rawNam);
LPCVOID find_process_memory(
    HANDLE procHan,
    LPCVOID ned,
    const size_t& nedSiz
);
int main(int argc, char** argv);
LPCVOID memmem(
    LPCVOID firMem,
    const size_t& firSiz,
    LPCVOID secMem,
    const size_t secSiz
);
farm_t read_farm(HANDLE procHan, LPCVOID headAdr);
racing_horse read_racing_horse
    (HANDLE procHan, LPCVOID headAdr, const unsigned& ind);
stallion_t read_stallion
    (HANDLE procHan, LPCVOID headAdr, const unsigned& ind);
std::string read_user_horse_name
    (HANDLE procHan, LPCVOID headAdr, const unsigned& ind);

template <class R>
Sortable_Table<R>::Sortable_Table(
    const int& x,
    const int& y,
    const int& wid,
    const int& hei,
    const compare_t* comps
) : Fl_Table_Row(x, y, wid, hei), comps_(comps), col_(-1) {
    callback(self_callback, this);
}

template <class R>
void Sortable_Table<R>::sort() {
    sorted_records.clear();
    for (R& rec : records) sorted_records.push_back(&rec);
    if (col_ >= 0) {
        compare_t comp = comps_[col_];
        if (ord_) comp = std::not2(comp);
        std::sort(sorted_records.begin(), sorted_records.end(), comp);
    }
}

template <class R>
void Sortable_Table<R>::self_callback(Fl_Widget* wid, void* some) {
    Sortable_Table* tab = (Sortable_Table*)(some);
    if (tab->callback_context() == CONTEXT_COL_HEADER &&
        Fl::event_button1()
    ) {
        if (tab->col_ != tab->callback_col()) {
            tab->col_ = tab->callback_col();
            tab->ord_ = 0;
        } else tab->ord_ ^= 1;
        tab->sort();
        tab->redraw();
    }
}

void Racing_Horse_Table::draw_cell
    (TableContext context, int R, int C, int X, int Y, int W, int H)
{
    using LAB = std::function<std::string(const racing_horse_record&)>;
    static const LAB LABS[19] = {
        [] (const racing_horse_record& rec) -> std::string {
            return rec.name;
        }, [] (const racing_horse_record& rec) -> std::string {
            return LOCATION_LABELS[std::min(rec.index / 10, 2u)];
        }, [] (const racing_horse_record& rec) -> std::string {
            return SEX_LABELS[rec.horse.sex()];
        }, [] (const racing_horse_record& rec) -> std::string {
            std::string lab;
            if (rec.horse.age < 9) lab = AGE_LABELS[rec.horse.age];
            else lab = Sprintf("%d", rec.horse.age + 1);
            return lab;
        }, [] (const racing_horse_record& rec) -> std::string {
            return COLOR_LABELS[rec.horse.color()];
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d+%d", rec.horse.max_speed,
                rec.horse.late_bloomer_bonus());
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.speed);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d%+04d", rec.horse.max_stamina,
                rec.horse.mare_complement(rec.father));
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.stamina);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.max_guts);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.guts);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.max_temper);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.temper);
        }, [] (const racing_horse_record& rec) -> std::string {
            return GROWTH_TYPE_LABELS[rec.horse.growth_type()];
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%02X", rec.horse.growth);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.dart);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%d", rec.horse.health);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%+04d", rec.horse.weight * 2);
        }, [] (const racing_horse_record& rec) -> std::string {
            return Sprintf("%02X", rec.horse.condition);
        },
    };
    switch (context) {
    case CONTEXT_STARTPAGE:
        fl_font(FL_COURIER, 11);
        break;
    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
        fl_color(FL_BLACK);
        fl_draw(
            RHT_COL_HEADER_LABELS[C].c_str(), X, Y, W, H, FL_ALIGN_CENTER
        );
        fl_pop_clip();
        break;
    case CONTEXT_CELL:
        fl_push_clip(X, Y, W, H);
        fl_color(row_selected(R) ? selection_color() : FL_WHITE);
        fl_rectf(X, Y, W, H);
        fl_color(FL_BLACK);
        fl_draw(
            LABS[C](*sorted_records[R]).c_str(),
            X,
            Y,
            W,
            H,
            RHT_COL_ALIGNS[C]
        );
        fl_color(color()); 
        fl_rect(X, Y, W, H);
        fl_pop_clip();
        break;
    }
}

void Process_Table::draw_cell
    (TableContext context, int R, int C, int X, int Y, int W, int H)
{
    using LAB = std::function<std::string(const PROCESSENTRY32&)>;
    static const LAB LABS[2] = {
        [] (const PROCESSENTRY32& ent) -> std::string {
            return ent.szExeFile;
        }, [] (const PROCESSENTRY32& ent) -> std::string {
            return Sprintf("%08x", ent.th32ProcessID);
        },
    };
    switch (context) {
    case CONTEXT_STARTPAGE:
        fl_font(FL_COURIER, 11);
        break;
    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
        fl_color(FL_BLACK);
        fl_draw
            (PT_COL_HEADER_LABELS[C].c_str(), X, Y, W, H, FL_ALIGN_CENTER);
        fl_pop_clip();
        break;
    case CONTEXT_CELL:
        fl_push_clip(X, Y, W, H);
        fl_color(row_selected(R) ? selection_color() : FL_WHITE);
        fl_rectf(X, Y, W, H);
        fl_color(FL_BLACK);
        fl_draw(
            LABS[C](*sorted_records[R]).c_str(),
            X,
            Y,
            W,
            H,
            PT_COL_ALIGNS[C]
        );
        fl_color(color()); 
        fl_rect(X, Y, W, H);
        fl_pop_clip();
        break;
    }
}

#endif

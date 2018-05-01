#include "dsexmon.h"
#include <Fl/Fl.H>
#include <Fl/Fl_ask.H>
#include <windows.h>
#include <tlhelp32.h>
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

const std::string APP_NAME = "ダビスタEXモニタ";
const std::string RHT_COL_HEADER_LABELS[19] = {
    "馬名",
    "繁用",
    "性",
    "齢",
    "毛色",
    "xSp",
    "Sp",
    "xSt",
    "St",
    "xGt",
    "Gt",
    "xTm",
    "Tm",
    "型",
    "成",
    "ダ",
    "健",
    "体重",
    "調",
};
const int RHT_COL_WIDTH[19] = {
    90,
    30,
    17,
    17,
    30,
    40,
    25,
    54,
    25,
    25,
    25,
    25,
    25,
    17,
    19,
    15,
    15,
    33,
    19,
};
const Fl_Align RHT_COL_ALIGNS[19] = {
    FL_ALIGN_LEFT,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
    FL_ALIGN_CENTER,
};
const std::string PROCESS_DIALOG_LABEL = "プロセスを開く";
const std::string PT_COL_HEADER_LABELS[2] = {"プロセス名", "プロセスID"};
const int PT_COL_WIDTH[2] = {130, 60};
const Fl_Align PT_COL_ALIGNS[2] = {FL_ALIGN_LEFT, FL_ALIGN_CENTER};
const BYTE DSEX_HEADER[16] = {
    0x8F, 0xAC, 0x93, 0x63, 0x95, 0x94, 0x07, 0x00,
    0x91, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x04, 0x28,
};
const std::string NAME_LETTERS[128] = {
    "？", "ア", "イ", "ウ", "エ", "オ", "カ", "キ",
    "ク", "ケ", "コ", "サ", "シ", "ス", "セ", "ソ",
    "タ", "チ", "ツ", "テ", "ト", "ナ", "ニ", "ヌ",
    "ネ", "ノ", "ハ", "ヒ", "フ", "ヘ", "ホ", "マ",
    "ミ", "ム", "メ", "モ", "ヤ", "ユ", "ヨ", "ワ",
    "ン", "ラ", "リ", "ル", "レ", "ロ", "ァ", "ィ",
    "ゥ", "ェ", "ォ", "ャ", "ュ", "ョ", "ッ", "ー",
    "？", "？", "パ", "ピ", "プ", "ペ", "ポ", "？",
    "？", "？", "？", "？", "？", "？", "ガ", "ギ",
    "グ", "ゲ", "ゴ", "ザ", "ジ", "ズ", "ゼ", "ゾ",
    "ダ", "ヂ", "ヅ", "デ", "ド", "？", "？", "？",
    "？", "？", "バ", "ビ", "ブ", "ベ", "ボ", "？",
    "？", "？", "？", "？", "？", "？", "？", "？",
    "？", "？", "？", "？", "？", "？", "？", "？",
    "？", "？", "？", "？", "？", "？", "？", "？",
    "？", "？", "？", "？", "？", "？", "？", "？",
};
const std::string LOCATION_LABELS[3] = {"美浦", "栗東", "牧場"};
const std::string SEX_LABELS[3] = {"牡", "牝", "騙"};
const std::string AGE_LABELS[9] =
    {"当", "２", "３", "４", "５", "６", "７", "８", "９"};
const std::string COLOR_LABELS[8] =
    {"栗毛", "鹿毛", "黒鹿", "栃栗", "青鹿", "芦毛", "青毛", "白毛"};
const std::string GROWTH_TYPE_LABELS[5] = {"早", "普", "晩", "？", "持"};

unsigned stallion_t::achievement() const {
    return (status >> 28) & 0x3;
}

unsigned stallion_t::color() const {
    return (status >> 8) & 0x7;
}

unsigned stallion_t::dart() const {
    return (status >> 18) & 0x3;
}

unsigned stallion_t::distance_lower() const {
    return (status >> 11) & 0x7;
}

unsigned stallion_t::distance_upper() const {
    return (status >> 14) & 0xf;
}

unsigned stallion_t::fee() const {
    return status & 0xff;
}

unsigned stallion_t::growth_type() const {
    return (status >> 20) & 0x3;
}

unsigned stallion_t::guts() const {
    return (status >> 24) & 0x3;
}

unsigned stallion_t::health() const {
    return (status >> 26) & 0x3;
}

unsigned stallion_t::stability() const {
    return (status >> 30) & 0x3;
}

unsigned stallion_t::temper() const {
    return (status >> 22) & 0x3;
}

unsigned racing_horse::color() const {
    return (sex_color >> 2) & 0x7;
}

unsigned racing_horse::growth_type() const {
    return growth >> 4;
}

unsigned racing_horse::late_bloomer_bonus() const {
    unsigned bon = 0;
    if ((growth >> 3) == 0x5) bon = max_speed / 25;
    return bon;
}

int racing_horse::mare_complement(const stallion_t& fat) const {
    int com = 0;
    if (sex()) com = std::min(int(max_temper) - int(max_stamina), 0) + 
        int(fat.distance_lower() + fat.distance_upper()) - 6;
    return com;
}

unsigned racing_horse::sex() const {
    return sex_color & 0x3;
}

Main_Window::Main_Window() :
    Fl_Double_Window(565, 242, APP_NAME.c_str()), process_handle(NULL)
{
    procBut_ = std::make_shared<Fl_Button>
        (0, 0, 565, 20, "ダビスタEXを実行しているプロセスを開く");
    procBut_->labelfont(FL_COURIER);
    procBut_->labelsize(11);
    procBut_->callback(procBut_callback, this);
    racHorTab_ = std::make_shared<Racing_Horse_Table>
        (0, 20, 565, 222, this);
    end();
    resizable(*racHorTab_);
    procDia_ = std::make_shared<Process_Dialog>(this);
    Fl::add_timeout(0.0, timer_callback, this);
}

void Main_Window::load(const DWORD& procId) {
    const HANDLE procHan = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!procHan) throw std::runtime_error(Sprintf("%s failed.(%d)",
        "OpenProcess", GetLastError()));
    const auto procHanExit = std::make_shared<scope_exit>(
        [procHan] () {CloseHandle(procHan);});
    const LPVOID dsexHeadAdr = LPVOID(find_process_memory
        (procHan, DSEX_HEADER, sizeof(DSEX_HEADER)));
    if (dsexHeadAdr) {
        process_handle = procHan;
        procHanExit_ = procHanExit;
        dsex_header_address = dsexHeadAdr;
        update();
    } else fl_alert("プロセスメモリ内にダビスタEXが見つかりません。");
}

void Main_Window::timer_callback(void* some) {
    Main_Window* win = (Main_Window*)(some);
    if (win->process_handle) win->update();
    Fl::repeat_timeout(1.0, timer_callback, win);
}

void Main_Window::update() {
    static char lab[256];
    Racing_Horse_Table* tab = racHorTab_.get();
    Process_Dialog* dia = procDia_.get();
    try {
        farm = read_farm(process_handle, dsex_header_address);
        if (std::string(farm.magic, farm.magic + 4) != "DSEX")
            throw std::runtime_error(Sprintf("farm not found."));
        std::string nam;
        if (*farm.name) nam = ansi_to_utf8(std::string(
            farm.name, farm.name + 8)) + "牧場";
        else nam = "牧場なし";
        std::sprintf(lab, "%s(%s)", APP_NAME.c_str(), nam.c_str());
        label(lab);
        tab->records.clear();
        for (unsigned i = 0; i < 35; ++i) {
            if (farm.racing[i] >= 0) {
                const racing_horse hor = read_racing_horse(
                    process_handle,
                    dsex_header_address,
                    farm.racing[i]
                );
                if (hor.name) {
                    tab->records.push_back(
                        racing_horse_record{
                            i,
                            hor,
                            read_user_horse_name(
                                process_handle,
                                dsex_header_address,
                                hor.name
                            ), read_stallion(
                                process_handle,
                                dsex_header_address,
                                hor.father
                            )
                        }
                    );
                }
            }
        }
        tab->sort();
        tab->rows(tab->records.size());
        tab->redraw();
    } catch (const std::runtime_error& err) {
        fl_alert(
            "データの読込みに失敗しました。\n"
            "モニタリングを中断します。");
        process_handle = NULL;
        label(APP_NAME.c_str());
        tab->records.clear();
        tab->rows(0);
    }
}

void Main_Window::procBut_callback(Fl_Widget* wid, void* some) {
    Main_Window* win = (Main_Window*)(some);
    Process_Dialog* dia = win->procDia_.get();
    try {
        dia->load();
        dia->position(
            win->x() + (win->w() - dia->w()) / 2,
            win->y() + (win->h() - dia->h()) / 2
        );
        dia->show();
    } catch (const std::runtime_error& err) {
        fl_alert("予期せぬエラーが発生しました。\n%s", err.what());
    }
}

Racing_Horse_Table::Racing_Horse_Table(
    const int& x,
    const int& y,
    const int& wid,
    const int& hei,
    Main_Window* mainWin
) :
    Sortable_Table<racing_horse_record>(x, y, wid, hei, comps_),
    mainWin_(mainWin)
{
    cols(19);
    col_header(1);
    col_header_height(20);
    col_resize(1);
    for (int C = 0; C < cols(); ++C) col_width(C, RHT_COL_WIDTH[C]);
    row_height_all(20);
    type(SELECT_SINGLE);
    selection_color(FL_YELLOW);
    end();
}

const Sortable_Table<racing_horse_record>::compare_t
    Racing_Horse_Table::comps_[19] =
{
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->name < rhs->name;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->index < rhs->index;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.sex() < rhs->horse.sex();},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.age < rhs->horse.age;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.color() < rhs->horse.color();},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.max_speed < rhs->horse.max_speed;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.speed < rhs->horse.speed;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.max_stamina < rhs->horse.max_stamina;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.stamina < rhs->horse.stamina;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.max_guts < rhs->horse.max_guts;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.guts < rhs->horse.guts;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.max_temper < rhs->horse.max_temper;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.temper < rhs->horse.temper;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.growth_type() < rhs->horse.growth_type();},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.growth < rhs->horse.growth;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.dart < rhs->horse.dart;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.health < rhs->horse.health;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.weight < rhs->horse.weight;},
    [] (racing_horse_record* lhs, racing_horse_record* rhs) -> bool
    {return lhs->horse.condition < rhs->horse.condition;},
};

Process_Dialog::Process_Dialog(Main_Window* mainWin) :
    Fl_Window(210, 242, PROCESS_DIALOG_LABEL.c_str()), mainWin_(mainWin)
{
    procTab_ = std::make_shared<Process_Table>(0, 0, 210, 222);
    okBut_ = std::make_shared<Fl_Return_Button>(0, 222, 105, 20, "OK");
    okBut_->labelfont(FL_COURIER);
    okBut_->labelsize(11);
    okBut_->callback(okBut_callback, this);
    canBut_ = std::make_shared<Fl_Button>(105, 222, 110, 20, "キャンセル");
    canBut_->labelfont(FL_COURIER);
    canBut_->labelsize(11);
    canBut_->callback(canBut_callback, this);
    end();
    resizable(*procTab_);
    set_modal();
}

void Process_Dialog::load() {
    HANDLE han = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (han == INVALID_HANDLE_VALUE)
        throw std::runtime_error(Sprintf("%s failed.(%d)",
            "CreateToolhelp32Snapshot", GetLastError()));
    const scope_exit hanExt([&han] () {CloseHandle(han);});
    PROCESSENTRY32 ent{sizeof(PROCESSENTRY32)};
    if (!Process32First(han, &ent))
        throw std::runtime_error(Sprintf("%s failed.(%d)",
            "Process32First", GetLastError()));
    procTab_->records.clear();
    for (;;) {
        procTab_->records.push_back(ent);
        if (!Process32Next(han, &ent)) {
            if (GetLastError() == ERROR_NO_MORE_FILES) break;
            throw std::runtime_error(Sprintf("%s failed.(%d)",
                "Process32Next", GetLastError()));
        }
    }
    procTab_->sort();
    procTab_->rows(procTab_->records.size());
    procTab_->redraw();
}

void Process_Dialog::okBut_callback(Fl_Widget* wid, void* some) {
    Process_Dialog* dia = (Process_Dialog*)(some);
    Process_Table* tab = dia->procTab_.get();
    int row = 0;
    for (; row < tab->rows(); ++row) {
        if (tab->row_selected(row)) break;
    }
    if (row < tab->rows()) {
        try {
            dia->hide();
            dia->mainWin_->load(tab->sorted_records[row]->th32ProcessID);
        } catch (const std::runtime_error& err) {
            fl_alert("予期せぬエラーが発生しました。\n%s", err.what());
        }
    } else fl_alert("プロセスを選んでください。");
}

void Process_Dialog::canBut_callback(Fl_Widget* wid, void* some) {
    Process_Dialog* dia = (Process_Dialog*)(some);
    dia->hide();
}

Process_Table::Process_Table
    (const int& x, const int& y, const int& wid, const int& hei) :
        Sortable_Table(x, y, wid, hei, comps_)
{
    cols(2);
    col_header(1);
    col_header_height(20);
    col_resize(1);
    for (int C = 0; C < 2; ++C) col_width(C, PT_COL_WIDTH[C]);
    row_height_all(20);
    type(SELECT_SINGLE);
    selection_color(FL_YELLOW);
    end();
}

const Sortable_Table<PROCESSENTRY32>::compare_t
    Process_Table::comps_[2] =
{
    [] (PROCESSENTRY32* lhs, PROCESSENTRY32* rhs) -> bool
    {return std::string(lhs->szExeFile) < std::string(rhs->szExeFile);},
    [] (PROCESSENTRY32* lhs, PROCESSENTRY32* rhs) -> bool
    {return lhs->th32ProcessID < rhs->th32ProcessID;},
};

scope_exit::scope_exit(const std::function<void()>& proc) : proc_(proc) {}

scope_exit::~scope_exit() {
    proc_();
}

std::string Sprintf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    return buf;
}

std::string ansi_to_utf8(const std::string& ansi) {
    const int utf16Len = MultiByteToWideChar
        (CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
    if (utf16Len == 0) throw std::runtime_error(Sprintf("%s failed.(%d)",
        "MultiByteToWideChar", GetLastError()));
    const std::shared_ptr<wchar_t> utf16
        (new wchar_t[utf16Len + 1], std::default_delete<wchar_t[]>());
    if (MultiByteToWideChar
        (CP_ACP, 0, ansi.c_str(), -1, utf16.get(), utf16Len) != utf16Len
    ) throw std::runtime_error(Sprintf("%s failed.(%d)",
        "MultiByteToWideChar", GetLastError()));
    const int utf8Len = WideCharToMultiByte
        (CP_UTF8, 0, utf16.get(), -1, NULL, 0, NULL, NULL);
    if (utf8Len == 0) throw std::runtime_error(Sprintf("%s failed.(%d)",
        "WideCharToMultiByte", GetLastError()));
    const std::shared_ptr<char> utf8
        (new char[utf8Len * 2], std::default_delete<char[]>());
    if (WideCharToMultiByte
        (CP_UTF8, 0, utf16.get(), -1, utf8.get(), utf8Len, NULL, NULL)
            != utf8Len
    ) throw std::runtime_error(Sprintf("%s failed.(%d)",
        "WideCharToMultiByte", GetLastError()));
    return utf8.get();
}

std::string decode_horse_name(const std::string& rawNam) {
    std::string nam;
    for (unsigned i = 0; i < rawNam.length(); ++i) {
        const char rawLet = rawNam[i];
        if (rawLet <= 0) break;
        nam += NAME_LETTERS[rawLet];
    }
    return nam;
}

LPCVOID find_process_memory(
    HANDLE procHan,
    LPCVOID ned,
    const size_t& nedSiz
) {
    LPCVOID res = NULL;
    std::vector<BYTE> buf;
    MEMORY_BASIC_INFORMATION inf{0};
    while (VirtualQueryEx(
        procHan,
        LPCBYTE(inf.BaseAddress) + inf.RegionSize,
        &inf,
        sizeof(MEMORY_BASIC_INFORMATION)
    )) {
        if (inf.State == MEM_COMMIT &&
            inf.Protect == PAGE_READWRITE &&
            inf.RegionSize >= nedSiz
        ) {
            buf.resize(inf.RegionSize);
            if (ReadProcessMemory(
                procHan,
                inf.BaseAddress,
                &buf.front(),
                buf.size(),
                NULL
            )) {
                LPCBYTE p = LPCBYTE(memmem
                    (&buf.front(), buf.size(), ned, nedSiz));
                if (p) res = LPCBYTE(inf.BaseAddress) + (p - &buf.front());
            }
        }
    }
    return res;
}

int main(int argc, char** argv) {
    const auto win = std::make_shared<Main_Window>();
    win->show(argc, argv);
    return Fl::run();
}

LPCVOID memmem(
    LPCVOID firMem,
    const size_t& firSiz,
    LPCVOID secMem,
    const size_t secSiz
) {
    LPCBYTE firByts = LPCBYTE(firMem), secByts = LPCBYTE(secMem);
    LPCVOID res = NULL;
    if (firSiz && secSiz && firSiz >= secSiz) {
        LPCBYTE las = firByts + firSiz - secSiz;
        for (LPCBYTE cur = firByts; cur <= las; ++cur) {
            if (!std::memcmp(cur, secByts, secSiz)) {
                res = cur;
                break;
            }
        }
    }
    return res;
}

farm_t read_farm(HANDLE procHan, LPCVOID headAdr) {
    farm_t farm;
    LPCVOID adr = LPCBYTE(headAdr) + 0x76b0;
    if (!ReadProcessMemory(procHan, adr, &farm, 80, NULL))
        throw std::runtime_error(Sprintf("%s failed.(%d)",
            "ReadProcessMemory", GetLastError()));
    return farm;
}

racing_horse read_racing_horse
    (HANDLE procHan, LPCVOID headAdr, const unsigned& ind)
{
    racing_horse hor;
    LPCVOID adr = LPCBYTE(headAdr) + 0xceb8 + 74 * ind;
    if (!ReadProcessMemory(procHan, adr, &hor, 74, NULL))
        throw std::runtime_error(Sprintf("%s failed.(%d)",
            "ReadProcessMemory", GetLastError()));
    return hor;
}

stallion_t read_stallion
    (HANDLE procHan, LPCVOID headAdr, const unsigned& ind)
{
    stallion_t stal;
    LPCVOID adr = LPCBYTE(headAdr) + 0x18e0 + 34 * ind;
    if (!ReadProcessMemory(procHan, adr, &stal, 34, NULL))
        throw std::runtime_error(Sprintf("%s failed.(%d)",
            "ReadProcessMemory", GetLastError()));
    return stal;
}

std::string read_user_horse_name
    (HANDLE procHan, LPCVOID headAdr, const unsigned& ind)
{
    std::string nam;
    nam.resize(9);
    LPCVOID adr = LPCBYTE(headAdr) + 0xaf00 + 9 * ind;
    if (!ReadProcessMemory(procHan, adr, &nam[0], 9, NULL))
        throw std::runtime_error(Sprintf("%s failed.(%d)",
            "ReadProcessMemory", GetLastError()));
    return decode_horse_name(nam);
}

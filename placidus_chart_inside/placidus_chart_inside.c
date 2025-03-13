/* 
  placidus_chart.c - 占星盤計算程序
  基於Swiss Ephemeris

  從輸入的出生日期、時間和地點計算行星位置、宮位和相位
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <direct.h>  /* 用於創建目錄 */
#include <sys/stat.h>  /* 用於檢查目錄是否存在 */
#include "swisseph-master_inside/swephexp.h"   /* Swiss Ephemeris header */

#define PLANETS_COUNT 10  /* 太陽到冥王星 */
#define ANGLES_COUNT 2    /* 上升點和天頂 */
#define EXTRA_POINTS_COUNT 2 /* 北交點和幸運點 */
#define HOUSES_COUNT 12   /* 12宮 */
#define ASPECTS_COUNT 5   /* 合相、三分相、六分相、二分相、四分相 */
#define EPHE_PATH "./swisseph-master_inside/ephe" /* 星曆表路徑 */
#define PI 3.14159265358979323846

/* 星座名稱 */
static const char *sign_names[] = {
    "牡羊座", "金牛座", "雙子座", "巨蟹座", "獅子座", "處女座",
    "天秤座", "天蠍座", "射手座", "摩羯座", "水瓶座", "雙魚座"
};

/* 行星名稱 */
static const char *planet_names[] = {
    "太陽", "月亮", "水星", "金星", "火星", "木星",
    "土星", "天王星", "海王星", "冥王星", "上升點", "天頂點",
    "北交點", "幸運點"
};

/* 宮位名稱 */
static const char *house_names[] = {
    "第1宮(命宮)", "第2宮(財帛宮)", "第3宮(兄弟宮)", "第4宮(田宅宮)",
    "第5宮(男女宮)", "第6宮(奴僕宮)", "第7宮(夫妻宮)", "第8宮(疾厄宮)",
    "第9宮(遷移宮)", "第10宮(官祿宮)", "第11宮(福德宮)", "第12宮(玄秘宮)"
};

/* 相位名稱 */
static const char *aspect_names[] = {
    "合相", "三分相", "六分相", "二分相", "四分相"
};

/* 相位角度 */
static const double aspect_angles[] = {
    0.0, 120.0, 60.0, 180.0, 90.0
};

/* 相位容許度 */
static const double aspect_orbs[] = {
    8.0, 8.0, 6.0, 8.0, 6.0
};

/* 行星守護的星座 (古典) */
static const int ruling_signs[][2] = {
    {4, -1},     /* 太陽 - 獅子座 */
    {3, -1},     /* 月亮 - 巨蟹座 */
    {2, 5},      /* 水星 - 雙子座、處女座 */
    {1, 6},      /* 金星 - 金牛座、天秤座 */
    {0, 7},      /* 火星 - 牡羊座、天蠍座 */
    {8, 11},     /* 木星 - 射手座、雙魚座 */
    {9, 10},     /* 土星 - 摩羯座、水瓶座 */
    {-1, -1},    /* 天王星 (現代) */
    {-1, -1},    /* 海王星 (現代) */
    {-1, -1}     /* 冥王星 (現代) */
};

/* 行星顏色和符號 (用於繪圖) */
static const char *planet_colors[] = {
    "#F9D71C", /* 太陽 - 金黃色 */
    "#E6E6E6", /* 月亮 - 銀白色 */
    "#BFD9BF", /* 水星 - 淺綠色 */
    "#F4C6D4", /* 金星 - 粉紅色 */
    "#E34234", /* 火星 - 紅色 */
    "#90A4AE", /* 木星 - 藍灰色 */
    "#673AB7", /* 土星 - 紫色 */
    "#00A0B0", /* 天王星 - 青藍色 */
    "#1565C0", /* 海王星 - 深藍色 */
    "#3E2723", /* 冥王星 - 深棕色 */
    "#FF5722", /* 上升點 - 橙色 */
    "#795548", /* 天頂點 - 棕色 */
    "#607D8B", /* 北交點 - 藍灰色 */
    "#FFC107"  /* 幸運點 - 琥珀色 */
};

static const char *planet_symbols[] = {
    "&#9737;", /* 太陽 */
    "&#9789;", /* 月亮 */
    "&#9791;", /* 水星 */
    "&#9792;", /* 金星 */
    "&#9794;", /* 火星 */
    "&#9795;", /* 木星 */
    "&#9796;", /* 土星 */
    "&#9797;", /* 天王星 */
    "&#9798;", /* 海王星 */
    "&#9799;", /* 冥王星 */
    "ASC",     /* 上升點 */
    "MC",      /* 天頂點 */
    "&#9739;", /* 北交點 */
    "POF"      /* 幸運點 */
};

/* 星座顏色和符號 */
static const char *sign_colors[] = {
    "#E34234", /* 牡羊座 - 紅色 */
    "#8BC34A", /* 金牛座 - 綠色 */
    "#FFEB3B", /* 雙子座 - 黃色 */
    "#E6E6E6", /* 巨蟹座 - 銀白色 */
    "#F9D71C", /* 獅子座 - 金黃色 */
    "#BFD9BF", /* 處女座 - 淺綠色 */
    "#F4C6D4", /* 天秤座 - 粉紅色 */
    "#673AB7", /* 天蠍座 - 紫色 */
    "#00A0B0", /* 射手座 - 青藍色 */
    "#3E2723", /* 摩羯座 - 深棕色 */
    "#1565C0", /* 水瓶座 - 深藍色 */
    "#607D8B"  /* 雙魚座 - 藍灰色 */
};

static const char *sign_symbols[] = {
    "♈︎", /* 牡羊座 */
    "♉︎", /* 金牛座 */
    "♊︎", /* 雙子座 */
    "♋︎", /* 巨蟹座 */
    "♌︎", /* 獅子座 */
    "♍︎", /* 處女座 */
    "♎︎", /* 天秤座 */
    "♏︎", /* 天蠍座 */
    "♐︎", /* 射手座 */
    "♑︎", /* 摩羯座 */
    "♒︎", /* 水瓶座 */
    "♓︎"  /* 雙魚座 */
};

/* 相位顏色和符號 */
static const char *aspect_colors[] = {
    "#F44336", /* 合相 - 紅色 */
    "#4CAF50", /* 三分相 - 綠色 */
    "#2196F3", /* 六分相 - 藍色 */
    "#FF5722", /* 二分相 - 橙色 */
    "#9C27B0"  /* 四分相 - 紫色 */
};

static const char *aspect_symbols[] = {
    "☌", /* 合相 */
    "△", /* 三分相 */
    "⚹", /* 六分相 */
    "☍", /* 二分相 */
    "□"  /* 四分相 */
};

/* 結構: 行星或角度資料 */
typedef struct {
    int planet_num;
    double longitude;
    int sign;
    double sign_longitude;
    int house;
} PlanetInfo;

/* 結構: 宮位資料 */
typedef struct {
    double cusp_longitude;
    int sign;
    double sign_longitude;
    int ruler;
} HouseInfo;

/* 結構: 相位資料 */
typedef struct {
    int planet1;
    int planet2;
    int aspect_type;
    double orb;
    int applying;  /* 1 = 正在形成, 0 = 正在分離 */
} AspectInfo;

/* 函數聲明 */
int calculate_chart(int year, int month, int day, int hour, int minute, 
                  double longitude, double latitude, 
                  PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int ephemeris_type);
void display_chart(PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int aspect_count,
                  int year, int month, int day, int hour, int minute, double longitude, double latitude);
int calculate_aspects(PlanetInfo *planets, AspectInfo *aspects);
char *get_dms(double longitude, int type);
int estimate_timezone(double longitude);
double calculate_pars_fortunae(double sun_lon, double moon_lon, double asc_lon);
void generate_svg_chart(PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int aspect_count,
                       int year, int month, int day, int hour, int minute, double longitude, double latitude);
void create_folder(char *folder_name, int year, int month, int day, int hour, int minute, double longitude, double latitude);
void save_txt_data(char *folder_name, PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int aspect_count, 
                  int year, int month, int day, int hour, int minute, double longitude, double latitude);

int main(int argc, char *argv[])
{
    int year, month, day, hour, minute;
    double longitude, latitude;
    int aspect_count;
    char serr[256];
    int ephemeris_type = SEFLG_MOSEPH; // 預設使用內建近似計算
    char folder_name[256];

    /* 行星、宮位與相位資訊的陣列 */
    PlanetInfo planets[PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT];
    HouseInfo houses[HOUSES_COUNT];
    AspectInfo aspects[150]; /* 夠大的空間來保存所有可能的相位 */
    
    if (argc < 8) {
        printf("用法: %s <年> <月> <日> <時> <分> <經度> <緯度>\n", argv[0]);
        printf("例如: %s 1990 1 1 12 0 121.5 25.0\n", argv[0]);
        return 1;
    }
    
    /* 解析命令行參數 */
    year = atoi(argv[1]);
    month = atoi(argv[2]);
    day = atoi(argv[3]);
    hour = atoi(argv[4]);
    minute = atoi(argv[5]);
    longitude = atof(argv[6]);
    latitude = atof(argv[7]);
    
    /* 設定ephemeris路徑 */
    swe_set_ephe_path(EPHE_PATH);
    
    /* 檢查JPL星曆表優先順序：DE441 > DE431 > 瑞士星曆表 > Moshier */
    char ephe_path_full[512];
    /* 構建完整路徑以確保正確檢測 */
    sprintf(ephe_path_full, "%s/de441.eph", EPHE_PATH);
    FILE *jpl_file = fopen(ephe_path_full, "rb");
    if (jpl_file != NULL) {
        fclose(jpl_file);
        printf("找到JPL星曆表(DE441)，使用最高精確度計算\n");
        ephemeris_type = SEFLG_JPLEPH;
        swe_set_jpl_file(ephe_path_full);
    } else {
        sprintf(ephe_path_full, "%s/de431.eph", EPHE_PATH);
        jpl_file = fopen(ephe_path_full, "rb");
        if (jpl_file != NULL) {
            fclose(jpl_file);
            printf("找到JPL星曆表(DE431)，使用最高精確度計算\n");
            ephemeris_type = SEFLG_JPLEPH;
            swe_set_jpl_file(ephe_path_full);
        } else {
            /* 檢查瑞士星曆表 */
            sprintf(ephe_path_full, "%s/semo_00.se1", EPHE_PATH);
            FILE *swiss_file = fopen(ephe_path_full, "r");
            if (swiss_file != NULL) {
                fclose(swiss_file);
                printf("找到瑞士星曆表，使用精確計算\n");
                ephemeris_type = SEFLG_SWIEPH;
            } else {
                printf("未找到星曆表文件，使用內建近似計算 (嘗試路徑: %s)\n", ephe_path_full);
                ephemeris_type = SEFLG_MOSEPH;
            }
        }
    }
    
    /* 檢查星曆表文件 */
    if (swe_get_library_path(serr) != NULL) {
        printf("使用星曆表路徑: %s\n", serr);
    }
    
    /* 計算占星盤 */
    aspect_count = calculate_chart(year, month, day, hour, minute, longitude, latitude, 
                                  planets, houses, aspects, ephemeris_type);
    
    /* 顯示結果 */
    display_chart(planets, houses, aspects, aspect_count, year, month, day, hour, minute, longitude, latitude);
    
    /* 生成SVG星盤圖 */
    generate_svg_chart(planets, houses, aspects, aspect_count, year, month, day, hour, minute, longitude, latitude);
    
    /* 創建資料夾並保存資料 */
    sprintf(folder_name, "%d_%d_%d_%d_%d_%.2f_%.2f", year, month, day, hour, minute, longitude, latitude);
    create_folder(folder_name, year, month, day, hour, minute, longitude, latitude);
    
    /* 保存文本數據 */
    save_txt_data(folder_name, planets, houses, aspects, aspect_count, year, month, day, hour, minute, longitude, latitude);
    
    /* 複製HTML文件到資料夾 */
    char html_file[100];
    char html_copy[512];
    char copy_command[1024];
    sprintf(html_file, "chart_%d_%d_%d_%d_%d.html", year, month, day, hour, minute);
    sprintf(html_copy, "%s/%d_%d_%d_%d_%d_%.2f_%.2f.html", 
            folder_name, year, month, day, hour, minute, longitude, latitude);
    sprintf(copy_command, "copy \"%s\" \"%s\" >nul", html_file, html_copy);
    system(copy_command);
    printf("已複製HTML文件到: %s\n", html_copy);
    
    /* 關閉 */
    swe_close();
    
    return 0;
}

/* 估算時區 */
int estimate_timezone(double longitude) {
    /* 根據經度估算時區, 每15度為一個時區 */
    return (int)round(longitude / 15.0);
}

/* 本地時間轉換為世界時 */
double local_to_ut(double local_time, int timezone_offset) {
    /* 根據時區偏移量將本地時間轉換為世界時 */
    return fmod(local_time - timezone_offset + 24.0, 24.0);
}

/* 計算幸運點 (Pars Fortunae) */
double calculate_pars_fortunae(double sun_lon, double moon_lon, double asc_lon) {
    double fortune = asc_lon + moon_lon - sun_lon;
    // 確保經度在0-360度範圍內
    while (fortune < 0) fortune += 360.0;
    while (fortune >= 360.0) fortune -= 360.0;
    return fortune;
}

/* 計算占星盤 */
int calculate_chart(int year, int month, int day, int hour, int minute, 
                  double longitude, double latitude, 
                  PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int ephemeris_type)
{
    int i, planet;
    double jd_ut;
    double cusps[13], ascmc[10];
    double x[6];
    int retval, aspect_count;
    char serr[256];
    int timezone = estimate_timezone(longitude);
    double fortune_lon;
    int iflag = SEFLG_SPEED | ephemeris_type;
    
    /* 計算世界時的儒略日（考慮時區） */
    jd_ut = swe_julday(year, month, day, hour + minute / 60.0, SE_GREG_CAL);
    /* 將當地時間轉換為UTC */
    jd_ut -= timezone / 24.0;
    
    /* 計算宮位 (使用Placidus制) */
    retval = swe_houses(jd_ut, latitude, longitude, 'P', cusps, ascmc);
    if (retval < 0) {
        printf("宮位計算錯誤\n");
        return 0;
    }
    
    /* 存儲宮位資訊 */
    for (i = 0; i < HOUSES_COUNT; i++) {
        houses[i].cusp_longitude = cusps[i + 1];
        houses[i].sign = (int)(houses[i].cusp_longitude / 30.0);
        houses[i].sign_longitude = houses[i].cusp_longitude - houses[i].sign * 30.0;
        
        /* 找出宮主星 (使用古典守護) */
        for (planet = 0; planet < PLANETS_COUNT; planet++) {
            if (ruling_signs[planet][0] == houses[i].sign || 
                ruling_signs[planet][1] == houses[i].sign) {
                houses[i].ruler = planet;
                break;
            }
        }
    }
    
    /* 計算各行星位置 */
    for (i = 0; i < PLANETS_COUNT; i++) {
        retval = swe_calc_ut(jd_ut, i, iflag, x, serr);
        if (retval < 0) {
            printf("計算行星位置出錯: %s\n", serr);
            /* 嘗試使用內建近似計算 */
            iflag = SEFLG_SPEED | SEFLG_MOSEPH;
            retval = swe_calc_ut(jd_ut, i, iflag, x, serr);
            if (retval < 0) {
                printf("內建近似計算也失敗: %s\n", serr);
                return 0;
            }
        }
        
        planets[i].planet_num = i;
        planets[i].longitude = x[0];
        planets[i].sign = (int)(x[0] / 30.0);
        planets[i].sign_longitude = x[0] - planets[i].sign * 30.0;
        
        /* 確定行星所在宮位 */
        planets[i].house = 0;
        for (int h = 0; h < HOUSES_COUNT; h++) {
            int next_house = (h + 1) % HOUSES_COUNT;
            if (houses[h].cusp_longitude <= houses[next_house].cusp_longitude) {
                /* 一般情況 */
                if (planets[i].longitude >= houses[h].cusp_longitude && 
                    planets[i].longitude < houses[next_house].cusp_longitude) {
                    planets[i].house = h;
                    break;
                }
            } else {
                /* 跨越0度的情況 */
                if (planets[i].longitude >= houses[h].cusp_longitude || 
                    planets[i].longitude < houses[next_house].cusp_longitude) {
                    planets[i].house = h;
                    break;
                }
            }
        }
    }
    
    /* 計算ASC和MC */
    planets[PLANETS_COUNT].planet_num = PLANETS_COUNT;     /* ASC */
    planets[PLANETS_COUNT].longitude = ascmc[0];
    planets[PLANETS_COUNT].sign = (int)(ascmc[0] / 30.0);
    planets[PLANETS_COUNT].sign_longitude = ascmc[0] - planets[PLANETS_COUNT].sign * 30.0;
    planets[PLANETS_COUNT].house = 0;  /* ASC總是在第1宮 */
    
    planets[PLANETS_COUNT + 1].planet_num = PLANETS_COUNT + 1;  /* MC */
    planets[PLANETS_COUNT + 1].longitude = ascmc[1];
    planets[PLANETS_COUNT + 1].sign = (int)(ascmc[1] / 30.0);
    planets[PLANETS_COUNT + 1].sign_longitude = ascmc[1] - planets[PLANETS_COUNT + 1].sign * 30.0;
    planets[PLANETS_COUNT + 1].house = 9;  /* MC總是在第10宮 */
    
    /* 計算北交點 (Mean Node) */
    retval = swe_calc_ut(jd_ut, SE_MEAN_NODE, iflag, x, serr);
    if (retval < 0) {
        printf("計算北交點位置出錯: %s\n", serr);
        /* 嘗試使用內建近似計算 */
        iflag = SEFLG_SPEED | SEFLG_MOSEPH;
        retval = swe_calc_ut(jd_ut, SE_MEAN_NODE, iflag, x, serr);
        if (retval < 0) {
            printf("內建近似計算也失敗: %s\n", serr);
            return 0;
        }
    }
    
    planets[PLANETS_COUNT + 2].planet_num = PLANETS_COUNT + 2;  /* 北交點 */
    planets[PLANETS_COUNT + 2].longitude = x[0];
    planets[PLANETS_COUNT + 2].sign = (int)(x[0] / 30.0);
    planets[PLANETS_COUNT + 2].sign_longitude = x[0] - planets[PLANETS_COUNT + 2].sign * 30.0;
    
    /* 確定北交點所在宮位 */
    planets[PLANETS_COUNT + 2].house = 0;
    for (int h = 0; h < HOUSES_COUNT; h++) {
        int next_house = (h + 1) % HOUSES_COUNT;
        if (houses[h].cusp_longitude <= houses[next_house].cusp_longitude) {
            /* 一般情況 */
            if (planets[PLANETS_COUNT + 2].longitude >= houses[h].cusp_longitude && 
                planets[PLANETS_COUNT + 2].longitude < houses[next_house].cusp_longitude) {
                planets[PLANETS_COUNT + 2].house = h;
                break;
            }
        } else {
            /* 跨越0度的情況 */
            if (planets[PLANETS_COUNT + 2].longitude >= houses[h].cusp_longitude || 
                planets[PLANETS_COUNT + 2].longitude < houses[next_house].cusp_longitude) {
                planets[PLANETS_COUNT + 2].house = h;
                break;
            }
        }
    }
    
    /* 計算幸運點 (Pars Fortunae) */
    fortune_lon = calculate_pars_fortunae(planets[0].longitude, planets[1].longitude, planets[PLANETS_COUNT].longitude);
    
    planets[PLANETS_COUNT + 3].planet_num = PLANETS_COUNT + 3;  /* 幸運點 */
    planets[PLANETS_COUNT + 3].longitude = fortune_lon;
    planets[PLANETS_COUNT + 3].sign = (int)(fortune_lon / 30.0);
    planets[PLANETS_COUNT + 3].sign_longitude = fortune_lon - planets[PLANETS_COUNT + 3].sign * 30.0;
    
    /* 確定幸運點所在宮位 */
    planets[PLANETS_COUNT + 3].house = 0;
    for (int h = 0; h < HOUSES_COUNT; h++) {
        int next_house = (h + 1) % HOUSES_COUNT;
        if (houses[h].cusp_longitude <= houses[next_house].cusp_longitude) {
            /* 一般情況 */
            if (planets[PLANETS_COUNT + 3].longitude >= houses[h].cusp_longitude && 
                planets[PLANETS_COUNT + 3].longitude < houses[next_house].cusp_longitude) {
                planets[PLANETS_COUNT + 3].house = h;
                break;
            }
        } else {
            /* 跨越0度的情況 */
            if (planets[PLANETS_COUNT + 3].longitude >= houses[h].cusp_longitude || 
                planets[PLANETS_COUNT + 3].longitude < houses[next_house].cusp_longitude) {
                planets[PLANETS_COUNT + 3].house = h;
                break;
            }
        }
    }
    
    /* 計算相位 */
    aspect_count = calculate_aspects(planets, aspects);
    
    return aspect_count;
}

/* 計算相位 */
int calculate_aspects(PlanetInfo *planets, AspectInfo *aspects)
{
    int i, j, k, count = 0;
    double diff, orb;
    
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT - 1; i++) {
        for (j = i + 1; j < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; j++) {
            /* 計算兩行星間的角度差 */
            diff = fabs(planets[i].longitude - planets[j].longitude);
            if (diff > 180.0) diff = 360.0 - diff;
            
            /* 檢查是否符合任何相位 */
            for (k = 0; k < ASPECTS_COUNT; k++) {
                orb = fabs(diff - aspect_angles[k]);
                /* 如果在容許度內 */
                if (orb <= aspect_orbs[k]) {
                    aspects[count].planet1 = i;
                    aspects[count].planet2 = j;
                    aspects[count].aspect_type = k;
                    aspects[count].orb = orb;
                    
                    /* 判斷正在形成還是正在分離 */
                    if (((planets[i].longitude < planets[j].longitude) && 
                         (diff < 180.0)) ||
                        ((planets[i].longitude > planets[j].longitude) && 
                         (diff > 180.0))) {
                        aspects[count].applying = 1;  /* 正在形成 */
                    } else {
                        aspects[count].applying = 0;  /* 正在分離 */
                    }
                    
                    count++;
                }
            }
        }
    }
    
    return count;
}

/* 顯示計算結果 */
void display_chart(PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int aspect_count,
                  int year, int month, int day, int hour, int minute, double longitude, double latitude)
{
    int i;
    int timezone = estimate_timezone(longitude);
    double jd_ut = swe_julday(year, month, day, hour + minute / 60.0, SE_GREG_CAL) - timezone / 24.0;
    double local_time = hour + minute / 60.0;
    double delta_t = swe_deltat(jd_ut); // Delta T in days
    double sid_time;
    
    /* 計算恆星時 - 考慮時區調整 */
    sid_time = swe_sidtime(jd_ut);
    /* 調整恆星時以匹配當地時間 */
    sid_time = fmod(sid_time + longitude / 15.0, 24.0);
    
    printf("\n==== 基本資料 ====\n");
    printf("日期: %d-%02d-%02d %02d:%02d\n", year, month, day, hour, minute);
    printf("地理位置: 經度 %.2f, 緯度 %.2f\n", longitude, latitude);
    printf("時區: UTC%+d\n", timezone);
    printf("地球時: %.2f\n", local_time);
    printf("恆星時: %.2f\n", sid_time);
    printf("儒略日UT: %.6f\n", jd_ut);
    printf("儒略日TT: %.6f\n", jd_ut + delta_t);
    printf("Delta T: %.2f秒\n", delta_t * 86400.0); // 轉換為秒
    
    printf("\n==== 行星位置 ====\n");
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; i++) {
        printf("%-8s：%-10s %s，%s\n", 
               planet_names[i], 
               sign_names[planets[i].sign],
               get_dms(planets[i].sign_longitude, 0),
               house_names[planets[i].house]);
    }
    
    printf("\n===== 宮位資訊 =====\n");
    for (i = 0; i < HOUSES_COUNT; i++) {
        printf("%-16s：%-10s %s，宮主星：%-8s\n", 
               house_names[i], 
               sign_names[houses[i].sign],
               get_dms(houses[i].sign_longitude, 0),
               planet_names[houses[i].ruler]);
    }
    
    printf("\n==== 相位 ====\n");
    for (i = 0; i < aspect_count; i++) {
        printf("%-8s - %-8s：%-8s，%.2f%s\n", 
               planet_names[aspects[i].planet1], 
               planet_names[aspects[i].planet2],
               aspect_names[aspects[i].aspect_type],
               aspects[i].orb,
               aspects[i].applying ? "（形成中）" : "（分離中）");
    }
}

/* 將經度轉換為度分秒格式 */
char *get_dms(double longitude, int type)
{
    static char buf[20];
    int deg, min, sec;
    double fraction;
    
    deg = (int)longitude;
    fraction = longitude - deg;
    min = (int)(fraction * 60);
    fraction = fraction * 60 - min;
    sec = (int)(fraction * 60);
    
    if (type == 0) {
        /* 格式: 度°分' */
        sprintf(buf, "%02d°%02d'", deg, min);
    } else {
        /* 格式: 度°分'秒" */
        sprintf(buf, "%02d°%02d'%02d\"", deg, min, sec);
    }
    
    return buf;
}

/* 生成SVG星盤圖 */
void generate_svg_chart(PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int aspect_count,
                       int year, int month, int day, int hour, int minute, double longitude, double latitude)
{
    FILE *fp;
    int i, j;
    char filename[100];
    
    /* 建立檔案名稱 */
    sprintf(filename, "chart_%d_%d_%d_%d_%d.html", year, month, day, hour, minute);
    
    fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("無法建立檔案 %s\n", filename);
        return;
    }
    
    /* 寫入HTML和SVG頭部 */
    fprintf(fp, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(fp, "<meta charset=\"UTF-8\">\n");
    fprintf(fp, "<title>占星盤 - %d-%02d-%02d %02d:%02d</title>\n", year, month, day, hour, minute);
    fprintf(fp, "<style>\n");
    fprintf(fp, "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }\n");
    fprintf(fp, ".container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n");
    fprintf(fp, "h1, h2 { color: #333; }\n");
    fprintf(fp, ".chart-container { display: flex; flex-wrap: wrap; justify-content: space-between; }\n");
    fprintf(fp, ".chart { width: 600px; height: 600px; margin: 0 auto; }\n");
    fprintf(fp, ".info { width: 500px; margin: 0 auto; }\n");
    fprintf(fp, "table { width: 100%%; border-collapse: collapse; margin: 20px 0; }\n");
    fprintf(fp, "th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }\n");
    fprintf(fp, "th { background-color: #f2f2f2; }\n");
    fprintf(fp, ".aspect-grid { border-collapse: collapse; width: 100%%; }\n");
    fprintf(fp, ".aspect-grid td { width: 30px; height: 30px; text-align: center; font-size: 16px; }\n");
    fprintf(fp, ".sign-symbol { font-size: 26px; font-weight: bold; }\n");
    fprintf(fp, ".planet-symbol { font-size: 20px; font-weight: bold; }\n");
    fprintf(fp, ".degree-marks { fill: none; stroke: #ccc; stroke-width: 0.5; }\n");
    fprintf(fp, ".aspect-line { pointer-events: none; }\n");
    fprintf(fp, ".zodiac-ring { fill: white; stroke: #888; stroke-width: 1; }\n");
    fprintf(fp, "</style>\n");
    fprintf(fp, "</head>\n<body>\n");
    fprintf(fp, "<div class=\"container\">\n");
    fprintf(fp, "<h1>占星盤</h1>\n");
    fprintf(fp, "<p>日期: %d-%02d-%02d %02d:%02d</p>\n", year, month, day, hour, minute);
    fprintf(fp, "<p>地理位置: 經度 %.2f, 緯度 %.2f</p>\n", longitude, latitude);
    
    fprintf(fp, "<div class=\"chart-container\">\n");
    
    /* SVG星盤圖開始 */
    fprintf(fp, "<div class=\"chart\">\n");
    fprintf(fp, "<svg width=\"600\" height=\"600\" viewBox=\"0 0 600 600\">\n");
    
    int cx = 300;  /* 中心點x */
    int cy = 300;  /* 中心點y */
    int r1 = 270;  /* 外圓半徑 */
    int r2 = 240;  /* 黃道帶外圓半徑 */
    int r3 = 210;  /* 黃道帶內圓半徑 */
    int inner_r = 170; /* 內圓半徑，用於放置行星 */
    
    /* 畫背景和整體圓環 */
    fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"#FFFFFF\" stroke=\"#AAA\" stroke-width=\"1\" />\n", cx, cy, r1);
    
    /* 添加黃道帶背景 */
    fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" class=\"zodiac-ring\" />\n", cx, cy, r2);
    fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" class=\"zodiac-ring\" />\n", cx, cy, r3);
    
    /* 添加度數標記 */
    for (i = 0; i < 360; i += 5) {
        double angle = i * PI / 180.0;
        int len = (i % 30 == 0) ? 15 : ((i % 10 == 0) ? 10 : 5);
        
        int x1 = cx + (int)(r2 * sin(angle));
        int y1 = cy - (int)(r2 * cos(angle));
        int x2 = cx + (int)((r2 - len) * sin(angle));
        int y2 = cy - (int)((r2 - len) * cos(angle));
        
        fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" class=\"degree-marks\" />\n", x1, y1, x2, y2);
    }
    
    /* 畫宮位線 */
    for (i = 0; i < HOUSES_COUNT; i++) {
        double angle = houses[i].cusp_longitude * PI / 180.0;
        int x1 = cx + (int)(r3 * sin(angle));
        int y1 = cy - (int)(r3 * cos(angle));
        int x2 = cx + (int)(inner_r * sin(angle));
        int y2 = cy - (int)(inner_r * cos(angle));
        
        fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#666\" stroke-width=\"1\" stroke-dasharray=\"4,4\" />\n", 
                x1, y1, x2, y2);
        
        /* 添加宮位數字 - 在圓外 */
        int mid_angle = (int)((houses[i].cusp_longitude + houses[(i+1)%12].cusp_longitude) / 2);
        if (houses[i].cusp_longitude > houses[(i+1)%12].cusp_longitude) {
            mid_angle = (int)((houses[i].cusp_longitude + houses[(i+1)%12].cusp_longitude + 360) / 2);
            if (mid_angle >= 360) mid_angle -= 360;
        }
        
        double num_angle = mid_angle * PI / 180.0;
        int xnum = cx + (int)((r1 - 15) * sin(num_angle));
        int ynum = cy - (int)((r1 - 15) * cos(num_angle));
        
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-size=\"12\" text-anchor=\"middle\" dominant-baseline=\"middle\" fill=\"#333\">%d</text>\n", 
                xnum, ynum, i + 1);
    }
    
    /* 在黃道帶中繪製星座符號 - 每個星座置中 */
    for (i = 0; i < 12; i++) {
        double angle = (i * 30 + 15) * PI / 180.0;
        int x = cx + (int)((r2 + r3) / 2 * sin(angle));
        int y = cy - (int)((r2 + r3) / 2 * cos(angle));
        
        /* 為星座符號區域添加底色 */
        fprintf(fp, "<text x=\"%d\" y=\"%d\" class=\"sign-symbol\" fill=\"%s\" text-anchor=\"middle\" dominant-baseline=\"middle\">%s</text>\n", 
                x, y, sign_colors[i], sign_symbols[i]);
        
        /* 添加星座分隔線 */
        double sep_angle = i * 30 * PI / 180.0;
        int x1 = cx + (int)(r2 * sin(sep_angle));
        int y1 = cy - (int)(r2 * cos(sep_angle));
        int x2 = cx + (int)(r3 * sin(sep_angle));
        int y2 = cy - (int)(r3 * cos(sep_angle));
        
        fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#888\" stroke-width=\"1\" />\n", 
                x1, y1, x2, y2);
    }
    
    /* 中心圓 */
    fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"#FFFFFF\" stroke=\"#AAA\" stroke-width=\"1\" />\n", 
            cx, cy, inner_r);
    
    /* 計算行星分布以避免重疊 */
    double planet_angles[PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT];
    double planet_distance[PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT];
    
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; i++) {
        planet_angles[i] = planets[i].longitude;
        planet_distance[i] = inner_r * 0.75; /* 預設距離 */
    }
    
    /* 調整行星位置避免重疊 */
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; i++) {
        for (j = 0; j < i; j++) {
            double angle_diff = fabs(planet_angles[i] - planet_angles[j]);
            if (angle_diff > 180.0) angle_diff = 360.0 - angle_diff;
            
            if (angle_diff < 8.0) { /* 如果行星過近 */
                /* 將行星稍微移動以避免重疊 */
                planet_distance[i] = planet_distance[i] * 0.85;
                planet_distance[j] = planet_distance[j] * 1.15;
                
                /* 水平方向稍微分開 */
                planet_angles[i] += 2.0;
                planet_angles[j] -= 2.0;
                
                /* 確保角度在0-360度範圍內 */
                if (planet_angles[i] >= 360.0) planet_angles[i] -= 360.0;
                if (planet_angles[j] < 0.0) planet_angles[j] += 360.0;
            }
        }
    }
    
    /* 畫行星 */
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; i++) {
        double angle = planet_angles[i] * PI / 180.0;
        double dist = planet_distance[i];
        int x = cx + (int)(dist * sin(angle));
        int y = cy - (int)(dist * cos(angle));
        
        /* 繪製行星圓形背景 */
        fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"16\" fill=\"white\" stroke=\"%s\" stroke-width=\"2\" />\n", 
                x, y, planet_colors[i]);
        
        /* 繪製行星符號 */
        fprintf(fp, "<text x=\"%d\" y=\"%d\" class=\"planet-symbol\" fill=\"%s\" text-anchor=\"middle\" dominant-baseline=\"middle\">%s</text>\n", 
                x, y, planet_colors[i], planet_symbols[i]);
        
        /* 繪製連接中心的線 */
        int line_x = cx + (int)(inner_r * sin(angle));
        int line_y = cy - (int)(inner_r * cos(angle));
        
        fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"%s\" stroke-width=\"1\" stroke-dasharray=\"2,2\" />\n", 
                x, y, line_x, line_y, planet_colors[i]);
    }
    
    /* 畫相位線 */
    for (i = 0; i < aspect_count; i++) {
        int p1 = aspects[i].planet1;
        int p2 = aspects[i].planet2;
        
        /* 僅畫主要行星之間的相位 */
        if (p1 < PLANETS_COUNT && p2 < PLANETS_COUNT) {
            double angle1 = planet_angles[p1] * PI / 180.0;
            double angle2 = planet_angles[p2] * PI / 180.0;
            double dist1 = planet_distance[p1];
            double dist2 = planet_distance[p2];
            
            int x1 = cx + (int)(dist1 * sin(angle1));
            int y1 = cy - (int)(dist1 * cos(angle1));
            int x2 = cx + (int)(dist2 * sin(angle2));
            int y2 = cy - (int)(dist2 * cos(angle2));
            
            /* 根據相位類型選擇線條樣式 */
            const char *color = aspect_colors[aspects[i].aspect_type];
            const char *dash = (aspects[i].aspect_type == 0 || aspects[i].aspect_type == 3) ? 
                            "" : "stroke-dasharray=\"5,5\"";
            
            fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"%s\" stroke-width=\"1.5\" %s class=\"aspect-line\" />\n", 
                    x1, y1, x2, y2, color, dash);
        }
    }
    
    fprintf(fp, "</svg>\n");
    fprintf(fp, "</div>\n");
    
    /* 信息面板 */
    fprintf(fp, "<div class=\"info\">\n");
    
    /* 行星表格 */
    fprintf(fp, "<h2>行星位置</h2>\n");
    fprintf(fp, "<table>\n");
    fprintf(fp, "<tr><th>行星</th><th>位置</th><th>宮位</th></tr>\n");
    
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; i++) {
        fprintf(fp, "<tr><td><span style=\"color: %s;\">%s %s</span></td><td>%s %02d°%02d'</td><td>%s</td></tr>\n", 
                planet_colors[i], planet_symbols[i], planet_names[i],
                sign_names[planets[i].sign], 
                (int)planets[i].sign_longitude, 
                (int)((planets[i].sign_longitude - (int)planets[i].sign_longitude) * 60),
                house_names[planets[i].house]);
    }
    
    fprintf(fp, "</table>\n");
    
    /* 宮位表格 */
    fprintf(fp, "<h2>宮位資訊</h2>\n");
    fprintf(fp, "<table>\n");
    fprintf(fp, "<tr><th>宮位</th><th>位置</th><th>宮主星</th></tr>\n");
    
    for (i = 0; i < HOUSES_COUNT; i++) {
        fprintf(fp, "<tr><td>%s</td><td>%s %02d°%02d'</td><td>%s</td></tr>\n", 
                house_names[i], 
                sign_names[houses[i].sign],
                (int)houses[i].sign_longitude,
                (int)((houses[i].sign_longitude - (int)houses[i].sign_longitude) * 60),
                planet_names[houses[i].ruler]);
    }
    
    fprintf(fp, "</table>\n");
    
    /* 相位表格 */
    fprintf(fp, "<h2>相位資訊</h2>\n");
    fprintf(fp, "<table>\n");
    fprintf(fp, "<tr><th>行星1</th><th>行星2</th><th>相位</th><th>偏差</th><th>狀態</th></tr>\n");
    
    for (i = 0; i < aspect_count; i++) {
        fprintf(fp, "<tr><td>%s</td><td>%s</td><td><span style=\"color: %s;\">%s</span></td><td>%.2f°</td><td>%s</td></tr>\n", 
                planet_names[aspects[i].planet1],
                planet_names[aspects[i].planet2],
                aspect_colors[aspects[i].aspect_type],
                aspect_names[aspects[i].aspect_type],
                aspects[i].orb,
                aspects[i].applying ? "形成中" : "分離中");
    }
    
    fprintf(fp, "</table>\n");
    
    /* 相位網格 */
    fprintf(fp, "<h2>相位網格</h2>\n");
    fprintf(fp, "<style>\n");
    fprintf(fp, ".aspect-grid { border-collapse: collapse; width: 100%%; }\n");
    fprintf(fp, ".aspect-grid td { text-align: center; padding: 4px; font-size: 12px; }\n");
    fprintf(fp, ".aspect-grid .header { font-weight: bold; background-color: #f2f2f2; }\n");
    fprintf(fp, ".aspect-cell { position: relative; }\n");
    fprintf(fp, ".aspect-symbol { font-size: 18px; line-height: 1; }\n");
    fprintf(fp, ".aspect-degree { font-size: 10px; display: block; margin-top: 2px; }\n");
    fprintf(fp, ".aspect-applying { position: absolute; top: 0; right: 2px; font-size: 8px; }\n");
    fprintf(fp, "</style>\n");
    fprintf(fp, "<table class=\"aspect-grid\" border=\"1\">\n");
    
    /* 表頭行 - 行星符號 */
    fprintf(fp, "<tr class=\"header\"><td></td>\n");
    for (i = 0; i < PLANETS_COUNT; i++) {
        fprintf(fp, "<td style=\"color: %s;\">%s</td>\n", planet_colors[i], planet_symbols[i]);
    }
    fprintf(fp, "</tr>\n");
    
    /* 表格內容 */
    for (i = 0; i < PLANETS_COUNT; i++) {
        fprintf(fp, "<tr>\n");
        fprintf(fp, "<td class=\"header\" style=\"color: %s;\">%s</td>\n", planet_colors[i], planet_symbols[i]);
        
        for (j = 0; j < PLANETS_COUNT; j++) {
            if (j > i) {
                /* 查找這兩個行星之間的相位 */
                int found = 0;
                for (int k = 0; k < aspect_count; k++) {
                    if ((aspects[k].planet1 == i && aspects[k].planet2 == j) || 
                        (aspects[k].planet1 == j && aspects[k].planet2 == i)) {
                        /* 計算度數格式，例如 3°20'a 表示分離中，3°20's 表示形成中 */
                        int degrees = (int)aspects[k].orb;
                        int minutes = (int)((aspects[k].orb - degrees) * 60);
                        char applying_char = aspects[k].applying ? 'a' : 's';
                        
                        fprintf(fp, "<td class=\"aspect-cell\" style=\"color: %s;\">\n", 
                                aspect_colors[aspects[k].aspect_type]);
                        fprintf(fp, "  <div class=\"aspect-symbol\">%s</div>\n", aspect_symbols[aspects[k].aspect_type]);
                        fprintf(fp, "  <div class=\"aspect-degree\">%d°%02d'%c</div>\n", 
                                degrees, minutes, applying_char);
                        fprintf(fp, "</td>\n");
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    fprintf(fp, "<td></td>\n");
                }
            } else if (j < i) {
                /* 對稱位置填寫相同的相位 */
                int found = 0;
                for (int k = 0; k < aspect_count; k++) {
                    if ((aspects[k].planet1 == i && aspects[k].planet2 == j) || 
                        (aspects[k].planet1 == j && aspects[k].planet2 == i)) {
                        /* 計算度數格式，例如 3°20'a 表示分離中，3°20's 表示形成中 */
                        int degrees = (int)aspects[k].orb;
                        int minutes = (int)((aspects[k].orb - degrees) * 60);
                        char applying_char = aspects[k].applying ? 'a' : 's';
                        
                        fprintf(fp, "<td class=\"aspect-cell\" style=\"color: %s;\">\n", 
                                aspect_colors[aspects[k].aspect_type]);
                        fprintf(fp, "  <div class=\"aspect-symbol\">%s</div>\n", aspect_symbols[aspects[k].aspect_type]);
                        fprintf(fp, "  <div class=\"aspect-degree\">%d°%02d'%c</div>\n", 
                                degrees, minutes, applying_char);
                        fprintf(fp, "</td>\n");
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    fprintf(fp, "<td></td>\n");
                }
            } else {
                /* 對角線位置 (i == j) */
                fprintf(fp, "<td style=\"background-color: #f2f2f2;\">%s</td>\n", planet_symbols[i]);
            }
        }
        fprintf(fp, "</tr>\n");
    }
    
    fprintf(fp, "</table>\n");
    
    fprintf(fp, "</div>\n"); /* 結束info div */
    fprintf(fp, "</div>\n"); /* 結束chart-container div */
    
    /* 頁腳 */
    fprintf(fp, "<p style=\"text-align: center; margin-top: 20px; color: #666;\">由Swiss Ephemeris生成的占星盤 - %d-%02d-%02d %02d:%02d</p>\n", 
            year, month, day, hour, minute);
    
    fprintf(fp, "</div>\n");
    fprintf(fp, "</body>\n</html>\n");
    
    fclose(fp);
    
    printf("\n星盤圖已保存到文件: %s\n", filename);
}

/* 創建資料夾 */
void create_folder(char *folder_name, int year, int month, int day, int hour, int minute, double longitude, double latitude) {
    struct stat st = {0};
    
    /* 資料夾名稱在呼叫此函數前已創建 */
    
    /* 檢查目錄是否已存在 */
    if (stat(folder_name, &st) == -1) {
        /* 目錄不存在，創建它 */
        if (_mkdir(folder_name) != 0) {
            printf("無法創建目錄 %s\n", folder_name);
            return;
        }
        printf("已創建資料夾: %s\n", folder_name);
    } else {
        printf("資料夾已存在: %s\n", folder_name);
    }
}

/* 保存文本數據 */
void save_txt_data(char *folder_name, PlanetInfo *planets, HouseInfo *houses, AspectInfo *aspects, int aspect_count, 
                  int year, int month, int day, int hour, int minute, double longitude, double latitude) {
    FILE *fp;
    int i;
    char filename[512];
    
    /* 建立檔案名稱，保留2位小數 */
    sprintf(filename, "%s/%d_%d_%d_%d_%d_%.2f_%.2f.txt", 
            folder_name, year, month, day, hour, minute, longitude, latitude);
    
    fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("無法建立檔案 %s\n", filename);
        return;
    }
    
    /* 寫入基本資料 */
    fprintf(fp, "==== 基本資料 ====\n");
    fprintf(fp, "日期: %d-%02d-%02d %02d:%02d\n", year, month, day, hour, minute);
    fprintf(fp, "地理位置: 經度 %.2f, 緯度 %.2f\n", longitude, latitude);
    int timezone_offset = estimate_timezone(longitude);
    fprintf(fp, "時區: UTC%+d\n", timezone_offset);
    
    /* 計算世界時的儒略日（考慮時區） */
    double local_time = hour + minute / 60.0;
    double ut_time = local_to_ut(local_time, timezone_offset);
    double jd_ut = swe_julday(year, month, day, ut_time, SE_GREG_CAL);
    double delta_t = swe_deltat(jd_ut); // Delta T in days
    double sid_time;
    
    /* 計算恆星時 - 考慮時區調整 */
    sid_time = swe_sidtime(jd_ut);
    /* 調整恆星時以匹配當地時間 */
    sid_time = fmod(sid_time + longitude / 15.0, 24.0);
    
    fprintf(fp, "本地時間: %.2f\n", local_time);
    fprintf(fp, "世界時UT: %.2f\n", ut_time);
    fprintf(fp, "恆星時: %.2f\n", sid_time);
    fprintf(fp, "儒略日UT: %.6f\n", jd_ut);
    fprintf(fp, "儒略日TT: %.6f\n", jd_ut + delta_t);
    fprintf(fp, "Delta T: %.2f秒\n\n", delta_t * 86400.0); // 轉換為秒
    
    /* 寫入行星位置 */
    fprintf(fp, "==== 行星位置 ====\n");
    for (i = 0; i < PLANETS_COUNT + ANGLES_COUNT + EXTRA_POINTS_COUNT; i++) {
        fprintf(fp, "%-8s：%-10s %s，%s\n", 
               planet_names[i], 
               sign_names[planets[i].sign],
               get_dms(planets[i].sign_longitude, 0),
               house_names[planets[i].house]);
    }
    fprintf(fp, "\n");
    
    /* 寫入宮位資訊 */
    fprintf(fp, "===== 宮位資訊 =====\n");
    for (i = 0; i < HOUSES_COUNT; i++) {
        fprintf(fp, "%-16s：%-10s %s，宮主星：%-8s\n", 
               house_names[i], 
               sign_names[houses[i].sign],
               get_dms(houses[i].sign_longitude, 0),
               planet_names[houses[i].ruler]);
    }
    fprintf(fp, "\n");
    
    /* 寫入相位 */
    fprintf(fp, "==== 相位 ====\n");
    for (i = 0; i < aspect_count; i++) {
        fprintf(fp, "%-8s - %-8s：%-8s，%.2f%s\n", 
               planet_names[aspects[i].planet1],
               planet_names[aspects[i].planet2],
               aspect_names[aspects[i].aspect_type],
               aspects[i].orb,
               aspects[i].applying ? "形成中" : "分離中");
    }

    fclose(fp);
    printf("已保存文本數據到: %s\n", filename);
} 
#include <sin_cos_int.h>

int sin_cos_table[][2] ={
	{0, 1000000},
	{17452, 999847},
	{34899, 999390},
	{52335, 998629},
	{69756, 997564},
	{87155, 996194},
	{104528, 994521},
	{121869, 992546},
	{139173, 990268},
	{156434, 987688},
	{173648, 984807},
	{190809, 981627},
	{207911, 978147},
	{224951, 974370},
	{241921, 970295},
	{258819, 965925},
	{275637, 961261},
	{292371, 956304},
	{309017, 951056},
	{325568, 945518},
	{342020, 939692},
	{358367, 933580},
	{374606, 927183},
	{390731, 920504},
	{406736, 913545},
	{422618, 906307},
	{438371, 898794},
	{453990, 891006},
	{469471, 882947},
	{484809, 874619},
	{500000, 866025},
	{515038, 857167},
	{529919, 848048},
	{544639, 838670},
	{559192, 829037},
	{573576, 819152},
	{587785, 809017},
	{601815, 798635},
	{615661, 788010},
	{629320, 777146},
	{642787, 766044},
	{656059, 754709},
	{669130, 743144},
	{681998, 731353},
	{694658, 719339},
	{707106, 707106},
	{719339, 694658},
	{731353, 681998},
	{743144, 669130},
	{754709, 656059},
	{766044, 642787},
	{777146, 629320},
	{788010, 615661},
	{798635, 601815},
	{809017, 587785},
	{819152, 573576},
	{829037, 559192},
	{838670, 544639},
	{848048, 529919},
	{857167, 515038},
	{866025, 499999},
	{874619, 484809},
	{882947, 469471},
	{891006, 453990},
	{898794, 438371},
	{906307, 422618},
	{913545, 406736},
	{920504, 390731},
	{927183, 374606},
	{933580, 358367},
	{939692, 342020},
	{945518, 325568},
	{951056, 309016},
	{956304, 292371},
	{961261, 275637},
	{965925, 258819},
	{970295, 241921},
	{974370, 224951},
	{978147, 207911},
	{981627, 190808},
	{984807, 173648},
	{987688, 156434},
	{990268, 139173},
	{992546, 121869},
	{994521, 104528},
	{996194, 87155},
	{997564, 69756},
	{998629, 52335},
	{999390, 34899},
	{999847, 17452},
	{1000000, 0},
	{999847, -17452},
	{999390, -34899},
	{998629, -52335},
	{997564, -69756},
	{996194, -87155},
	{994521, -104528},
	{992546, -121869},
	{990268, -139173},
	{987688, -156434},
	{984807, -173648},
	{981627, -190809},
	{978147, -207911},
	{974370, -224951},
	{970295, -241921},
	{965925, -258819},
	{961261, -275637},
	{956304, -292371},
	{951056, -309017},
	{945518, -325568},
	{939692, -342020},
	{933580, -358367},
	{927183, -374606},
	{920504, -390731},
	{913545, -406736},
	{906307, -422618},
	{898794, -438371},
	{891006, -453990},
	{882947, -469471},
	{874619, -484809},
	{866025, -500000},
	{857167, -515038},
	{848048, -529919},
	{838670, -544639},
	{829037, -559192},
	{819152, -573576},
	{809017, -587785},
	{798635, -601815},
	{788010, -615661},
	{777146, -629320},
	{766044, -642787},
	{754709, -656058},
	{743144, -669130},
	{731353, -681998},
	{719339, -694658},
	{707106, -707106},
	{694658, -719339},
	{681998, -731353},
	{669130, -743144},
	{656058, -754709},
	{642787, -766044},
	{629320, -777145},
	{615661, -788010},
	{601815, -798635},
	{587785, -809017},
	{573576, -819152},
	{559192, -829037},
	{544639, -838670},
	{529919, -848048},
	{515038, -857167},
	{500000, -866025},
	{484809, -874619},
	{469471, -882947},
	{453990, -891006},
	{438371, -898794},
	{422618, -906307},
	{406736, -913545},
	{390731, -920504},
	{374606, -927183},
	{358367, -933580},
	{342020, -939692},
	{325568, -945518},
	{309017, -951056},
	{292371, -956304},
	{275637, -961261},
	{258819, -965925},
	{241921, -970295},
	{224951, -974370},
	{207911, -978147},
	{190809, -981627},
	{173648, -984807},
	{156434, -987688},
	{139173, -990268},
	{121869, -992546},
	{104528, -994521},
	{87155, -996194},
	{69756, -997564},
	{52336, -998629},
	{34899, -999390},
	{17452, -999847},
	{0, -1000000},
	{-17452, -999847},
	{-34899, -999390},
	{-52335, -998629},
	{-69756, -997564},
	{-87155, -996194},
	{-104528, -994521},
	{-121869, -992546},
	{-139173, -990268},
	{-156434, -987688},
	{-173648, -984807},
	{-190808, -981627},
	{-207911, -978147},
	{-224951, -974370},
	{-241921, -970295},
	{-258819, -965925},
	{-275637, -961261},
	{-292371, -956304},
	{-309016, -951056},
	{-325568, -945518},
	{-342020, -939692},
	{-358367, -933580},
	{-374606, -927183},
	{-390731, -920504},
	{-406736, -913545},
	{-422618, -906307},
	{-438371, -898794},
	{-453990, -891006},
	{-469471, -882947},
	{-484809, -874619},
	{-500000, -866025},
	{-515037, -857167},
	{-529919, -848048},
	{-544639, -838670},
	{-559192, -829037},
	{-573576, -819152},
	{-587785, -809016},
	{-601815, -798635},
	{-615661, -788010},
	{-629320, -777145},
	{-642787, -766044},
	{-656059, -754709},
	{-669130, -743144},
	{-681998, -731353},
	{-694658, -719339},
	{-707106, -707106},
	{-719339, -694658},
	{-731353, -681998},
	{-743144, -669130},
	{-754709, -656059},
	{-766044, -642787},
	{-777146, -629320},
	{-788010, -615661},
	{-798635, -601815},
	{-809017, -587785},
	{-819152, -573576},
	{-829037, -559192},
	{-838670, -544639},
	{-848048, -529919},
	{-857167, -515037},
	{-866025, -499999},
	{-874619, -484809},
	{-882947, -469471},
	{-891006, -453990},
	{-898794, -438370},
	{-906307, -422618},
	{-913545, -406736},
	{-920504, -390731},
	{-927183, -374606},
	{-933580, -358367},
	{-939692, -342020},
	{-945518, -325568},
	{-951056, -309017},
	{-956304, -292371},
	{-961261, -275637},
	{-965925, -258818},
	{-970295, -241921},
	{-974370, -224951},
	{-978147, -207911},
	{-981627, -190808},
	{-984807, -173648},
	{-987688, -156434},
	{-990268, -139173},
	{-992546, -121869},
	{-994521, -104528},
	{-996194, -87155},
	{-997564, -69756},
	{-998629, -52336},
	{-999390, -34899},
	{-999847, -17452},
	{-1000000, 0},
	{-999847, 17452},
	{-999390, 34899},
	{-998629, 52336},
	{-997564, 69756},
	{-996194, 87155},
	{-994521, 104528},
	{-992546, 121869},
	{-990268, 139173},
	{-987688, 156434},
	{-984807, 173648},
	{-981627, 190808},
	{-978147, 207911},
	{-974370, 224951},
	{-970295, 241921},
	{-965925, 258819},
	{-961261, 275637},
	{-956304, 292371},
	{-951056, 309017},
	{-945518, 325568},
	{-939692, 342020},
	{-933580, 358367},
	{-927183, 374606},
	{-920504, 390731},
	{-913545, 406736},
	{-906307, 422618},
	{-898794, 438371},
	{-891006, 453990},
	{-882947, 469471},
	{-874619, 484809},
	{-866025, 499999},
	{-857167, 515037},
	{-848048, 529919},
	{-838670, 544639},
	{-829037, 559192},
	{-819152, 573576},
	{-809017, 587785},
	{-798635, 601815},
	{-788010, 615661},
	{-777146, 629320},
	{-766044, 642787},
	{-754709, 656058},
	{-743144, 669130},
	{-731353, 681998},
	{-719339, 694658},
	{-707106, 707106},
	{-694658, 719339},
	{-681998, 731353},
	{-669130, 743144},
	{-656059, 754709},
	{-642787, 766044},
	{-629320, 777146},
	{-615661, 788010},
	{-601815, 798635},
	{-587785, 809016},
	{-573576, 819151},
	{-559192, 829037},
	{-544638, 838670},
	{-529919, 848048},
	{-515038, 857167},
	{-500000, 866025},
	{-484809, 874619},
	{-469471, 882947},
	{-453990, 891006},
	{-438371, 898794},
	{-422618, 906307},
	{-406736, 913545},
	{-390731, 920504},
	{-374606, 927183},
	{-358368, 933580},
	{-342020, 939692},
	{-325568, 945518},
	{-309016, 951056},
	{-292371, 956304},
	{-275637, 961261},
	{-258818, 965925},
	{-241921, 970295},
	{-224951, 974370},
	{-207911, 978147},
	{-190809, 981627},
	{-173647, 984807},
	{-156434, 987688},
	{-139173, 990268},
	{-121869, 992546},
	{-104528, 994521},
	{-87155, 996194},
	{-69756, 997564},
	{-52335, 998629},
	{-34899, 999390},
	{-17452, 999847},
};
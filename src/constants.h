#include <map>
#include <string>

const std::string SETTINGS_DIR_PATH = "../settings/";
const std::string AUXDATA_DIR_PATH = "../auxdata/";
const std::string ENCDATA_DIR_PATH = "../encdata/";

const std::string FHE_CONTEXT_SETTING_FILE_PATH =
  SETTINGS_DIR_PATH + "contextsetting.txt";
const std::string FHE_CONTEXT_FILE_PATH = SETTINGS_DIR_PATH + "context.bin";
const std::string FHE_PK_FILE_PATH = SETTINGS_DIR_PATH + "pk.bin";
const std::string FHE_SK_FILE_PATH = SETTINGS_DIR_PATH + "sk.bin";
const std::string DBBASICS_FILE_PATH = SETTINGS_DIR_PATH + "dbbasics.bin";
const std::string MED_INV_FILE_PATH = AUXDATA_DIR_PATH + "med.inv";
const std::string SIDE_INV_FILE_PATH = AUXDATA_DIR_PATH + "side.inv";

namespace csvcolumns
{
constexpr int ID_IDX = 0;
constexpr int MEDICINE_ID_IDX = 1;
constexpr int SYMPTOM_ID_IDX = 3;
constexpr int AGE_IDX = 11;
constexpr int GENDER_IDX = 12;
} // namespace csvcolumns

enum class Gender
{
    MALE = 0,
    FEMALE = 1,
    OTHER = 2
};

const std::map<int, Gender> GENDER_INT_MAP{
  {1, Gender::MALE}, {2, Gender::FEMALE}, {3, Gender::OTHER}};

const std::map<char, Gender> GENDER_CHAR_MAP{
  {'m', Gender::MALE}, {'f', Gender::FEMALE}, {'o', Gender::OTHER}};

#define FIND_NOMATCH    "nomatch"
#define FIND_NODATA     "nodata"

class ProfilesManager{
    public:
        ProfilesManager();
        void begin();
        void createProfile(char * name, int16_t wh);
        String findMatch(int wh);
        String getStoredProfiles();
        static void clearAllProfiles();
        void updateAppName(int zeroBasedindex, char * name);
    private:
        int nextFreeAddress;
        void updateNextFreeAddress();
};
        
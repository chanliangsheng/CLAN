#ifndef FA_H
#define FA_H
#include <pa.h>

class Fa : public Pa {
public:
    Fa();
    Fa(float mz, float intensity, float score, std::shared_ptr<DatabaseRecord> database_record);
};

#endif // FA_H

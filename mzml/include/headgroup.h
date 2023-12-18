#ifndef HEADGROUP_H
#define HEADGROUP_H

class Headgroup {
public:
    Headgroup();
    Headgroup(float mz, float intensity, float score);

public:
    void SetMz(float mz);
    void SetIntensity(float intensity);
    void SetScore(float score);
    float GetMz();
    float GetIntensity();
    float GetScore();

private:
    float m_mz;
    float m_intensity;
    float m_score;
};

#endif // HEADGROUP_H

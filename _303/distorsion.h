#ifndef DISTORSION_H_
#define DISTORSION_H_

class Distorsion {
  private:
  float _driveRange;
  float _drive;
  int _currentDrive;

  public:

  Distorsion(float d) {
    _driveRange = d;
    setDrive(0.0f);
  }

  void setDrive(float d) {
    _drive = d;
    _currentDrive = 1 + _driveRange * _drive;
  }

  int next(int in) {
    in *= _currentDrive;
    if(in > 127)
      in = 127;
    if(in < -128)
      in = -128;
    return in;
  }
};

#endif /* DISTORSION_H_ */

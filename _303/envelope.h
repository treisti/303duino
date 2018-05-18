#ifndef ENVELOPE_H_
#define ENVELOPE_H_

class Envelope {
  private:
  
  float _attackRange;
  float _decayRange;
  float _releaseRange;
  
  float _attack;
  float _decay;
  float _sustain;
  float _release;

  int _phase;
  int _step;
  float _value;

  public:

  Envelope(float a, float d, float r) {
    _attackRange = a;
    _decayRange = d;
    _releaseRange = r;

    _phase = 0;
    _step = 0;
    _value = 0.0f;

    _attack = 0.0f;
    _decay = 0.0f;
    _sustain = 0.0f;
    _release = 0.0f;
  }

  void setAttack(float a) {
    _attack = a;
  }

  void setDecay(float d) {
    _decay = d;
  }

  void setSustain(float s) {
    _sustain = s;
  }

  void setRelease(float r) {
    _release = r;
  }

  void start() {
    _phase = 1;
    _step = 0;
    _value = 0.0f;
  }

  void end() {
    if(_phase == 1 || _phase == 2) {
      if(_phase == 1) {
        _value = _step / (_attack * _attackRange);
      } else {
        _value = 1.0f - _step / (_decay * _decayRange) * (1.0f - _sustain);
      }
      if(_sustain > 0.0f) {
        _step = _release * _releaseRange * (1.0f - _value / _sustain);
        _phase = 4;
      } else {
        _step = 0;
        _phase = 0;
      }
    } else if(_phase = 3) {
      _step = 0;
      _phase = 4;
    }
  }

  void update() {
    switch(_phase) {
      case 1: // Attack
        if(_attack != 0) {
          _value = _step / (_attack * _attackRange);
          _step ++;
        }
        if(_value >= 1.0f || _attack == 0) {
           _phase ++;
           _step = 0;
        }
        break;
      case 2: // Decay 
        _value = 1.0f - _step / (_decay * _decayRange) * (1.0f - _sustain);
        _step ++;
        if(_value <= _sustain) {
          _phase ++;
        }
        break;
      case 3: // Sustain
        _value = _sustain;
        break;
      case 4: // Release
        _value = _sustain * (1.0f - _step / (_release * _releaseRange));
        if(_value <= 0.0f) {
          _phase ++;
        }
        break;
      default:
        _phase = 0;
        _value = 0.0f;
    }
  }

  int next(int in) {
    return _value * in;
  }

  float get() {
    return _value;
  }
};

#endif /* ENVELOPE_H_ */

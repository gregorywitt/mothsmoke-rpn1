

class ModeIndicator{
  public:
  
  bool updateNeeded;
  bool deg;
  static const int xPos = 185;
  static const int height = 16;

  ModeIndicator(){
    updateNeeded = true;
    deg = true;
  }
  
  void show(){
    updateNeeded = true;
  }

};

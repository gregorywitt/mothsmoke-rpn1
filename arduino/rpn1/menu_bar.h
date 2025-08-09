
class MenuBar{
  public:
  
  struct MenuItem{
    int bitmapOffset;
    void (*action)();
  };

  MenuItem* currentMenu;
  MenuItem blankMenu[4];
  MenuItem inverseMenu[4];
  MenuItem moreMenu[4];

  bool updateNeeded;
  static const int bitmapSize = 186; 

  MenuBar(){
    updateNeeded = true;
    currentMenu = blankMenu;    
  }
  
};
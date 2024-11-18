#ifndef __WASTE_DETECTOR__
#define __WASTE_DETECTOR__

class WasteDetector {
public:
  WasteDetector(int trigPin, int echoPin);
  void init();
  int getWasteLevel(); // Returns the waste distance level
  bool isFull(int threshold); // Determines if the waste has reached full capacity

private:
  int trigPin;
  int echoPin;
};

#endif

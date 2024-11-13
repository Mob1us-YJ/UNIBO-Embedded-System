#ifndef __WASTE_DETECTOR__
#define __WASTE_DETECTOR__

class WasteDetector {
public:
  WasteDetector(int trigPin, int echoPin);
  void init();
  int getWasteLevel(); // 返回废物的距离水平
  bool isFull(int threshold); // 判断废物是否达到满载状态

private:
  int trigPin;
  int echoPin;
};

#endif

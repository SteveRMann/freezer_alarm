void beginSerial() {
#ifndef ESP01
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println(F(SKETCH_NAME));
  Serial.println(F(SKETCH_VERSION));
  Serial.println();
#endif
}

# Senior-Design-TempID-Project
Work in Progress Application: Temperature Identification Device that monitors user's temperature and displays to others as a form of contact tracing

Personal Progress made on files: MainActivity.kt, TempDisplay.kt, TempIDStart.ino

MainActivity.kt: Main bluetooth scanner meant to discover the BlueFruit compatible device (e.g TempID00X)/(See example image)
https://drive.google.com/file/d/1Sl5MljbYzxgSk6NKEtE_rvYIA1PMia-c/view?usp=sharing

TempDisplay.kt: After connecting device user can subscribe to temperature/battery notifications to display on graph and log (See example image)
https://drive.google.com/file/d/1Sl5MljbYzxgSk6NKEtE_rvYIA1PMia-c/view?usp=sharing

TempIDStart.ino: Displays temperature taken from temperature sensors on LCD and sends data over Bluetooth to Android app (See example setup)
https://drive.google.com/file/d/1PF3BqqhQ89prgv7pUqRYRUOuvVIARfjf/view?usp=sharing

https://drive.google.com/file/d/1Zr-LfLh-YRi5bRtmqHU0CsQreeOE02Mg/view?usp=sharing

Credit to Punchthrough for remaining .kt files: https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
Credit to Team0x0E for TempIDStart file: Jacob Schultz (Temperature Sensors), Brycen Hakes (LCD), Raunel Albiter (Bluetooth,NeoPixel,ADC)

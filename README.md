# AM2302 / DHT22 library for ESP32 (ESP-IDF)

This is an ESP32 (esp-idf) library for the AM2302 / DHT22 low cost temperature/humidity sensors.

Contrary to other available libraries like [https://github.com/gosouth/DHT22] this one uses GPIO interrupts (falling edge) to read the data and avoids blocking the CPU or other ISRs.

**USE**

See example/am2302_example_main.c

```C
#define AM2302_PIN 22

void callback(double temperature, double humidity) {
    printf("temperature: %.1f humidity: %.1f\n", temperature, humidity);
}

void app_main()
{
    init_am2302(AM2302_PIN, callback);

    int cnt = 0;
    while (1)
    {
        printf("cnt: %d\n", cnt++);
        read_am2302();
        vTaskDelay(30000 / portTICK_RATE_MS);
    }
}
```

**Copy/paste from AM2302/DHT22 Docu:**

DATA: Hum = 16 bits, Temp = 16 Bits, check-sum = 8 Bits

Example: MCU has received 40 bits data from AM2302 as
0000 0010 1000 1100 0000 0001 0101 1111 1110 1110
16 bits RH data + 16 bits T data + check sum

1) convert 16 bits RH data from binary system to decimal system, 0000 0010 1000 1100 → 652
Binary system Decimal system: RH=652/10=65.2%RH

2) convert 16 bits T data from binary system to decimal system, 0000 0001 0101 1111 → 351
Binary system Decimal system: T=351/10=35.1°C

When highest bit of temperature is 1, it means the temperature is below 0 degree Celsius. 
Example: 1000 0000 0110 0101, T= minus 10.1°C: 16 bits T data

3) Check Sum=0000 0010+1000 1100+0000 0001+0101 1111=1110 1110 Check-sum=the last 8 bits of Sum=11101110

Signal & Timings:

The interval of whole process must be beyond 2 seconds.

To request data from DHT:

1) Sent low pulse for > 1~10 ms (MILI SEC)
2) Sent high pulse for > 20~40 us (Micros).
3) When DHT detects the start signal, it will pull low the bus 80us as response signal, 
   then the DHT pulls up 80us for preparation to send data.
4) When DHT is sending data to MCU, every bit's transmission begin with low-voltage-level that last 50us, 
   the following high-voltage-level signal's length decide the bit is "1" or "0".
	0: 26~28 us
	1: 70 us

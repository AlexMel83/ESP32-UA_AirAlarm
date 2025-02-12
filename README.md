# Air Alarm UA - ESP32

## Опис проекту

**Air Alarm UA** - це система стеження за повітряною тривогою на основі **ESP32**. Вона забезпечує автономний моніторинг повітряних тривог в вибраному регіоні та керування сигналізацією (наприклад, світлодіод чи сирена).

## Можливості

🔔 Отримує дані про повітряні тривоги з API Ukraine Alarm

🛡️ Контролює реле для підключення сигналізації

🌟 Підсвічує статус тривоги світлодіодом на другому піні.

💡 Працює через Wi-Fi. Змінна GREEN_LED горить постійно при втраті зв'язку та міготить якшо він є.

## Встановлення

### Крок 1: Клонування репозиторію

git clone https://github.com/AlexMel83/ESP32-UA_AirAlarm.git
cd ESP32-UA_AirAlarm

### Крок 2: Налаштування config.h

Щоб проект працював, потрібно перейменувати файл src/config.h.example в src/config.h.

Відкрийте config.h та змініть налаштування згідно вашої мережі:

#define WIFI*SSID "ВАШ_SSID"
#define WIFI_PASSWORD "ВАШ_WiFi*ПАРОЛЬ"
#define API*KEY "ВАШ_API_KEY" //який можете отримати залишивши тут заявку https://api.ukrainealarm.com/
#define API_REGION_ID "ВАШ_ID*РЕГІОНУ"
#define SERIAL_BAUD 115200 //змініть на вашу швидкість порта
const uint8_t local_ip[] = {192, 168, 1, 100};
const uint8_t gateway[] = {192, 168, 1, 1};

якщо потрібно то змініть піни
#define GREEN_LED 12 // Перевірте, чи підходить пін
#define RELAY 2 // Пін реле з підсвіткою вбудованого синього лед

### Крок 3: Завантаження коду на ESP32

Збірка та завантаження коду може бути здійснена через **PlatformIO** або **Arduino IDE**.

🛠️ Якщо у вас е пропозиції щодо покращення проекту, залишайте пул-реквести або пишіть в обговореннях!

### Крок 4: Завантаження вебсторінки на ESP32

За допомогою буль якого FTP клієнта завантажте вебсторінку для ручного керування реле та отримання інформації про тривоги

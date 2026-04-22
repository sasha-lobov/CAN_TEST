# CAN_TEST
Тестирование протокола DroneCAN

*Собирал решение в Oracle VM на Ubuntu 24.04 LTS

## Архитектура решения
Решение состоит из двух независимых компонентов, взаимодействующих через виртуальный CAN-интерфейс:
### Publisher (C/SocketCAN):
- Реализует логику микроконтроллера (STM32).
- Выполняет ручную сериализацию структуры BatteryInfo согласно DSDL спецификации (Little-Endian, плотная упаковка бит).
- Реализует транспортный уровень DroneCAN: формирование 29-битных CAN ID, фрагментация сообщений (Multi-frame Transfer), управление битами SOF/EOF, Toggle и счетчиком Transfer ID.
- Отправка данных осуществляется через стандартный Linux SocketAPI (socket, bind, write).
### Validator (Python/PyCyphal logic):
- Сниффер шины vcan0.
- Десериализует сырые CAN-фреймы, собирает фрагментированные трансферы.
- Интерпретирует байтовые данные в физические величины (Вольты, Амперы) для верификации корректности протокола.

## Сборка и запуск

```bash
# Активация интерфейса
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

```bash
# Компиляция Паблишера
gcc -Wall -Wextra -std=c11 -o battery_tx battery_tx.c
```

```bash
# Запуск валидатора
python3 decode.py
```

```bash
# Запуск отправки
sudo ./battery_tx
```

## Результаты тестирования
При успешном выполнении в окне валидатора отображаются интерпретированные данные, соответствующие зашитым в код значениям:
```bash
Voltage: 11.10 V, Current: -2.50 A, Temp: 25.50 C
```

### Реализация на C:

(Publisher)
<img width="720" height="134" alt="5" src="https://github.com/user-attachments/assets/fb8cd904-da45-4afd-8ea6-03adbb5eefe1" />

(Validator)
<img width="650" height="158" alt="4" src="https://github.com/user-attachments/assets/98b1f28b-baa5-45ad-9772-0378d682bec0" />


### Чистая Python-версия реализации:

(Publisher)
<img width="548" height="110" alt="2" src="https://github.com/user-attachments/assets/d2b82906-d80f-4e7a-8c26-90e6cfe21c58" />


(Validator)
<img width="524" height="196" alt="3" src="https://github.com/user-attachments/assets/7a21be08-0122-4d0e-b679-74df90442f5e" />


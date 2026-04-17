import machine
import time
from sensor import (flash_led, connect_wifi, disconnect_wifi, logPrint,
                    dht22Get, tempWaterGet, interpolate2D, interpolate3D,
                    pumpLogic, pushToSocket, lire_tensions,
                    update_boot_counter, capacityGet, safe_call, errorNumSet)
import network
from secrets import secrets

led_out_green = machine.Pin(0, machine.Pin.OUT)
led_out_red   = machine.Pin(1, machine.Pin.OUT)

socketMessage = {}
socketMessage["aquaErrorNum"]   = 0
socketMessage["aquaPumpVal"]    = 0
socketMessage["aquaPumpTime"]   = 0   # initialisé pour éviter KeyError côté Grafana
socketMessage["dht22AquaTemp"]  = 0

monthTable = [1, 4, 7, 10, 12]
timeTable  = [0, 6, 9, 12, 15, 18, 24]

#             month        1,  4,  7, 10, 12
sleepTable = [[30, 22, 30, 22, 30],  #  0h
              [22, 15, 20, 15, 22],  #  6h
              [15, 10,  5, 10, 15],  #  9h
              [ 7,  5,  5,  5,  7],  # 12h
              [15, 10,  5, 10, 15],  # 15h
              [22, 15, 10, 15, 22],  # 18h
              [30, 22, 15, 22, 30]]  # 24h

#            month      1,  4,  7, 10, 12
pumpTable  = [1,  3,  4,  3,  1]

pumpDuration  = 120
deepSleepTime = 15   # valeur de secours si le calcul échoue
rtc = machine.RTC()
serialConnect = True

# Codes d'erreur pour safe_call dans le main (bits 6 et au-delà)
# Les bits 1-5 sont déjà utilisés par les fonctions internes de sensor.py.
ERR_WIFI     = 6   # bit 6  = 64
ERR_DHT22    = 7   # bit 7  = 128
ERR_WATER    = 8   # bit 8  = 256
ERR_TENSION  = 9   # bit 9  = 512
ERR_CAPACITY = 10  # bit 10 = 1024
ERR_INTERP   = 11  # bit 11 = 2048
ERR_PUMP     = 12  # bit 12 = 4096
ERR_PUSH     = 13  # bit 13 = 8192
ERR_DISC     = 14  # bit 14 = 16384

####### Main prog ############
try:
    boot_count = update_boot_counter(rtc)
    socketMessage["aquaBootCount"] = boot_count
    logPrint("Script start", serialConnect)
    flash_led(3, led_out_green)

    wlan = safe_call(connect_wifi, ERR_WIFI, socketMessage, serialConnect,
                     serialConnect)

    safe_call(dht22Get,      ERR_DHT22,    socketMessage, serialConnect, socketMessage, serialConnect)
    safe_call(tempWaterGet,  ERR_WATER,    socketMessage, serialConnect, socketMessage, serialConnect)
    safe_call(lire_tensions, ERR_TENSION,  socketMessage, serialConnect, socketMessage, serialConnect)
    safe_call(capacityGet,   ERR_CAPACITY, socketMessage, serialConnect, socketMessage, serialConnect)

    # Calcul de l'heure et du temps de sommeil — bloc try/except dédié
    # car il produit des variables locales (timeOfDay, pumpTime) nécessaires à la suite.
    timeOfDay = 0
    pumpTime  = []
    try:
        timeOfDay = time.gmtime()[3] + time.gmtime()[4] / 60
        month     = time.gmtime()[1]
        socketMessage["timeOfDay"] = timeOfDay
        socketMessage["month"]     = month

        deepSleepTime = int(interpolate3D(timeTable, monthTable, sleepTable, timeOfDay, month, serialConnect))
        pumpTimeNb    = int(round(interpolate2D(month, monthTable, pumpTable)))

        if socketMessage["dht22AquaTemp"] <= 2.0:
            pumpTimeNb = 0

        logPrint("deepSleepTime : %s" % deepSleepTime, serialConnect)
        logPrint("pumpTimeNb : %s" % pumpTimeNb, serialConnect)

        if pumpTimeNb == 4:
            pumpTime = [10, 12, 14, 16]
        elif pumpTimeNb == 3:
            pumpTime = [10, 12, 14]
        elif pumpTimeNb == 2:
            pumpTime = [12, 14]
        elif pumpTimeNb == 1:
            pumpTime = [12]
        else:
            pumpTime = []

        socketMessage["aquaSleepTime"] = deepSleepTime
        socketMessage["aquaPumpNb"]    = pumpTimeNb
    except Exception as e:
        logPrint("Erreur calcul temps/interpolation [err=%d]: %s" % (ERR_INTERP, str(e)), serialConnect)
        socketMessage["aquaErrorNum"] = errorNumSet(ERR_INTERP, socketMessage["aquaErrorNum"], serialConnect)
        # timeOfDay=0 et pumpTime=[] déjà initialisés : la pompe ne s'activera pas
        # deepSleepTime reste à sa valeur de secours (15 min)

    safe_call(pumpLogic,      ERR_PUMP, socketMessage, serialConnect,
              socketMessage, pumpTime, pumpDuration, timeOfDay, rtc, serialConnect)
    safe_call(pushToSocket,   ERR_PUSH, socketMessage, serialConnect,
              socketMessage, serialConnect)
    safe_call(disconnect_wifi, ERR_DISC, socketMessage, serialConnect,
              wlan, serialConnect)

    logPrint("Going to sleep for : %s min" % deepSleepTime, serialConnect)
    flash_led(3, led_out_red)

except Exception as e:
    logPrint("Critical error in main prog: %s" % str(e), serialConnect)

machine.deepsleep(int(deepSleepTime * 60 * 1_000))

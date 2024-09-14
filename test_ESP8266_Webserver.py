# ================================== IMPORTS ==================================

import time
import requests
import pvporcupine
from playsound import playsound
from pvrecorder import PvRecorder
import pvrhino
import random
import pygame

# ================================== DECLARATIONS =====================================

access_key = "tZd2lX4gcFDCnvykYlyl5gftK8dv9qZG8gaFO2ETkk7xwnxVpzvtLw=="

NODEMCU_IP = "http://192.168.167.17"  # Replace with NodeMCU IP after connecting

# ======================================= SOUNDS =======================================

follow_up_sounds = ["follow_1.mp3", "follow_2.mp3", "follow_3.mp3", "follow_4.mp3"]

welcome_sounds = ["welcome_1.mp3","welcome_2.mp3"]

again_sounds = ["didnt_get_1.mp3","didnt_get_2.mp3","didnt_get_3.mp3","didnt_get_4.mp3","didnt_get_5.mp3"]

oky_sounds = ["oky_1.mp3","oky_2.mp3","oky_3.mp3","oky_4.mp3","oky_5.mp3"]

error_sounds = ["error_1.mp3","error_2.mp3","error_3.mp3","error_4.mp3","error_5.mp3",]

# =============================================== PYGAME SOUND FUNCTION ==========================================

def sound_func(variable_name,file_path):

    rand = random.choice(variable_name)

    pygame.mixer.init()# remember that i have added this for stopping the loop for some time
    pygame.mixer.music.load(f"{file_path}\{rand}") 
    pygame.mixer.music.play()
    while pygame.mixer.music.get_busy():
        time.sleep(0.1)


# =============================================== REQUEST CONTROL ================================================

def control_appliance(appliance, number, status):
    try:
        # Define a timeout period (e.g., 5 seconds)
        timeout = 0.5
        # status = "off" if status == "on" else "on" # This is for ESP8266
        url = f"{NODEMCU_IP}/control"

        if appliance == "light" or appliance == "fan":

            if number in ["main",  "primary", "front", "first"]:
                number = 1
               

            elif number in ["table","secondary", "second", "bed", "back"]:
                number = 2

            elif number in ["every", "all", "both"]:
                payload_1 = {'appliance': appliance.upper() + "_1", 'state': status}
                requests.post(url, data=payload_1, timeout=timeout)
                payload_1 = {'appliance': appliance.upper() + "_2", 'state': status}
                requests.post(url, data=payload_1, timeout=timeout)
                
            payload = {'appliance': (appliance.upper() + f"_{number}"), 'state': status}
            print(payload)
            requests.post(url, data=payload, timeout=timeout)
    
    except requests.exceptions.Timeout:

        sound_func(error_sounds,"error")
        print("Request timed out. The URL might be unreachable or taking too long to respond.")
    
    except requests.exceptions.RequestException as e:
        print(f"Request failed: {e}")

    except Exception as e:
        print(f"An error occurred: {e}")



# =============================================== WAKE WORD FUNCTION ================================================

def wake_word_func():
    
    
    
    wake_status = True
    try:
        
        rand = random.choice(welcome_sounds)
        playsound(
                    f"welcome_back\{rand}")
        porcupine = pvporcupine.create(access_key=access_key,
                                        keyword_paths=["कल्पना_hi_windows_v3_0_0.ppn"],
                                        model_path="porcupine_params_hi.pv")
        rhino = pvrhino.create(access_key=access_key,
                                        context_path="CU_home_intents\CU_home_en_windows_v3_0_0.rhn")

        
# =============================================== WAKE WORD ================================================

        recorder = PvRecorder(device_index=-1,frame_length=porcupine.frame_length)
        


        while True:
            if wake_status:
                print("Waiting for the wake-Word....")
                recorder.start()
                wake_status=False
            audio_frame = recorder.read()
            keyword_index = porcupine.process(audio_frame)

            if keyword_index == 0:
                flag=True
                print("Wake-word - Detected")

                recorder.stop() # i put this before, because I was delaying the loop

                sound_func(follow_up_sounds,"follow_up")
                
                
# =============================================== INTENTS ================================================

                try:

                    start_time = time.time()
                    print("Speak command : ")
                    recorder.start()
                    
                    while (flag==True and ((time.time()-start_time)<=4)):
                
                        audio_frame = recorder.read()
                        is_finalized = rhino.process(audio_frame)
           
                     
                        if is_finalized:                  
                            inference = rhino.get_inference()
                            
                            if inference.is_understood:
                                slots = inference.slots
                                print(slots)
                                sound_func(oky_sounds,"ok")
                            
                                control_appliance(slots.get("appliance"),slots.get("number"), slots.get("status"))
                                flag=False
                                recorder.stop()
                       
                        if int(time.time()-start_time)==4 and flag == True:
                                sound_func(again_sounds,"didnt_get")
                                recorder.stop()

                
                finally:
                    wake_status=True
                    print("TIME OVER")

    finally:
        porcupine.delete()
        rhino.delete()
        recorder.stop()
        recorder.delete()

wake_word_func()

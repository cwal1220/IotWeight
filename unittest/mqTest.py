import paho.mqtt.client as mqtt

NOTIFY_TOPIC = "WEIGHT_221126/NOTIFY"
WEIGHT_TOPIC = "WEIGHT_221126/WEIGHT"

# subscriber callback
def on_message(client, userdata, message):
    print("message received ", str(message.payload.decode("utf-8")))
    print("message topic=", message.topic)
    print("message qos=", message.qos)
    print("message retain flag=", message.retain)


broker_address = "test.mosquitto.org"
client1 = mqtt.Client("client1") #구독자 이름
client1.connect
client1.connect(broker_address) # broker 주소 등록
client1.subscribe(NOTIFY_TOPIC) # 등록하고픈 토픽 지정
client1.subscribe(WEIGHT_TOPIC) # 등록하고픈 토픽 지정
client1.on_message = on_message
client1.loop_forever()

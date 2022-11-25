import paho.mqtt.client as mqtt

NOTIFY_TOPIC = "WEIGHT_221126/NOTIFY"
WEIGHT_TOPIC = "WEIGHT_221126/WEIGHT"

mqttc = mqtt.Client("client2")
mqttc.connect("test.mosquitto.org", 1883)
mqttc.publish(WEIGHT_TOPIC, "33")
mqttc.publish(NOTIFY_TOPIC, "44")
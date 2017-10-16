all: app

app: ApplicationLayer.c Packets.c utils.c
	gcc ApplicationLayer.c Packets.c utils.c -o app -Wall

clean: 
	rm -f app
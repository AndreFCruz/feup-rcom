all: app

app: ApplicationLayer.c LinkLayer.c Packets.c utils.c main.c
	gcc -W ApplicationLayer.c LinkLayer.c Packets.c utils.c main.c -o app

clean: 
	rm -f app
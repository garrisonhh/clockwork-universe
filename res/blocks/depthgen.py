#/usr/bin/env python
import pygame as pg

clamp = lambda x, a, b: min(max(x, a), b)

def main():
    pg.init()
    screen = pg.display.set_mode((32, 36))

    image = pg.Surface((32, 36))

    image.fill((0, 0, 255))

    # generate top
    for y in range(16):
        for x in range(16):
            if 8 - int(x / 2) <= y + 1:
                image.set_at((x, y), [int((float(y) / 15) * 255.0)] * 3)

    # generate left side
    for i in range(8):
        c = image.get_at((i * 2, 8 + i))

        for y in range(21):
            for x in range(2):
                image.set_at(
                    (i * 2 + x, y + 8 + i),
                    [c[0] - int((float(y) / 20) * (255.0 * 0.5))] * 3
                )

    # flip left side to right
    image.blit(pg.transform.flip(image.subsurface((0, 0, 16, 36)), 1, 0), (16, 0))

    pg.image.save(image, "BLOCKDEPTH.png")

    screen.fill((0, 0, 0))
    screen.blit(image, (0, 0))

    while pg.QUIT not in [event.type for event in pg.event.get()]:
        pg.display.flip()

if __name__ == '__main__':
    main()

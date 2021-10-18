
import time
import mandlebrot


class Mandlebrot_c_module:

    def __init__(self):
        """"""
        self.Xs = -2.0 
        self.Xe = 1.0
        self.Ys = -1.5
        self.Ye = 1.5

    def generate(self, w, h):
        """"""
        frame = bytearray(mandlebrot.mandlebrot_bytearray(w, h, self.Xs, self.Xe, self.Ys, self.Ye))


if __name__ == '__main__':
    print("Testing c module")

    now = time.time()

    mand = Mandlebrot_c_module()
    mand.generate(320, 320)

    print("Took ", time.time() - now, " secs")


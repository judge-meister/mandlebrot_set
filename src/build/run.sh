

./mandelbrot_main -f 80 -r $(cat ./centre_point.txt | grep real | cut -d= -f2) -i $(cat ./centre_point.txt | grep imag | cut -d= -f2) 


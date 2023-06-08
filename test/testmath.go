//
// first attempt at writing go

package main

import (
    "fmt"
    "math"
    //"math/big"
    "time"
)

func grayscale(it, itmax int) int {
    var idx float64 = 0.0
    if it < itmax {
        idx = math.Sqrt( float64(it)/float64(itmax) ) * 255
    }
    return int(idx)
}

func iterate_location(x0, y0 float64) int {
    var x float64 = 0.0
    var y float64 = 0.0
    var iteration int = 0
    var maxiter int = 1000
    var xtemp float64 = 0.0

    for (x*x + y*y <= 4) && (iteration < maxiter) {
        xtemp = x*x - y*y + x0
        y = 2*x*y + y0
        x = xtemp
        iteration++

    }
    return  grayscale(iteration, maxiter)
}

func mandelbrot_set(w, h int) {
    var Xs float64 = -2.0
    var Xe float64 = 1.0
    var Ys float64 = -1.5
    var Ye float64 = 1.5
    var x0, y0 float64
    //var color int

    for Ph := 0; Ph < h; Ph++ {
        for Pw := 0; Pw < w; Pw++ {
            x0 = ((float64(Pw)/float64(w))*(Xe-Xs))+Xs
            y0 = ((float64(Ph)/float64(h))*(Ye-Ys))+Ys

            iterate_location(x0, y0)
        }
        //fmt.Printf("row %d color %d", Ph, color)
    }
}


func main() {
    fmt.Println("Testing float64 in go")
    now := time.Now()

    mandelbrot_set(640, 640)

    t := time.Now()
    fmt.Printf("Took %s secs\n", t.Sub(now).String())
}


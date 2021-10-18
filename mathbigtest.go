//
// first attempt at writing go

package main

import (
    "fmt"
    "math"
    "math/big"
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
    const prec = 100
    x := 0.0
    y := 0.0
    var iteration int = 0
    var maxiter int = 1000
    xtemp := 0.0

    sq_x_plus_sq_y := 0.0
    for (sq_x_plus_sq_y <= 4) && (iteration < maxiter) {
        sq_x_plus_sq_y = x*x + y*y
        xtemp = sq_x_plus_sq_y + x0
        y = 2*x*y + y0
        x = xtemp
        iteration++

    }
    return  grayscale(iteration, maxiter)
}

func mandlebrot_set(w, h int) {
    const prec = 128
    Xs := new(big.Float).SetPrec(prec).SetInt64(-2)
    Xe := new(big.Float).SetPrec(prec).SetInt64(1)
    Ys := new(big.Float).SetPrec(prec).SetFloat64(-1.5)
    Ye := new(big.Float).SetPrec(prec).SetFloat64(1.5)
    x0 := new(big.Float).SetPrec(prec).SetInt64(0)
    y0 := new(big.Float).SetPrec(prec).SetInt64(0)
    
    a := new(big.Float).SetPrec(prec).SetInt64(0)
    b := new(big.Float).SetPrec(prec).SetInt64(0)
    //c := new(big.Float).SetPrec(prec).SetInt64(0)
    maxiter := 1000
            
    four := new(big.Float).SetPrec(prec).SetInt64(4)
    two := new(big.Float).SetPrec(prec).SetInt64(2)

    x := new(big.Float).SetPrec(prec).SetInt64(0)
    y := new(big.Float).SetPrec(prec).SetInt64(0)
    xtemp := new(big.Float).SetPrec(prec).SetInt64(0)

    for Ph := 0; Ph < h; Ph++ {
        for Pw := 0; Pw < w; Pw++ {
            //x0 = ((float64(Pw)/float64(w))*(Xe-Xs))+Xs
            a.Sub(Xe,Xs)
            b.SetFloat64((float64(Pw)/float64(w)))
            x0.Mul(a,b)
            x0.Add(x0,Xs)
            //y0 = ((float64(Ph)/float64(h))*(Ye-Ys))+Ys
            a.Sub(Ye,Ys)
            b.SetFloat64((float64(Ph)/float64(h)))
            y0.Mul(a,b)
            y0.Add(y0,Ys)

            x.SetFloat64(0.0)
            y.SetFloat64(0.0)

            iteration := 0
            xtemp.SetFloat64(0.0)

            sq_x_plus_sq_y := new(big.Float).SetPrec(prec).SetInt64(0)
            for (sq_x_plus_sq_y.Cmp(four) <= 0) && (iteration < maxiter) {
                //sq_x_plus_sq_y = x*x + y*y
                a.Mul(x,x)
                b.Mul(y,y)
                sq_x_plus_sq_y.Add(a,b)
                xtemp.Add(sq_x_plus_sq_y,x0)

                //y = 2*x*y + y0
                a.Mul(x,y)
                b.Mul(a,two)

                y.Add(b,y0)
                x = xtemp
                iteration++
            }
            //iterate_location(x0, y0)
        }
        //fmt.Printf("row %d color %d", Ph, color)
    }
}


func main() {
    fmt.Println("Testing float64 in go")
    now := time.Now()

    mandlebrot_set(640, 640)

    t := time.Now()
    fmt.Printf("Took %s secs\n", t.Sub(now).String())
}


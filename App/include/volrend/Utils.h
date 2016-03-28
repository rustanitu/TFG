#ifndef VOLREND_UTILS_H
#define VOLREND_UTILS_H

#include <glutils/GLTexture3D.h>|
#include <volrend/transferfunction.h>
#include <volrend/volume.h>

namespace vr
{
  gl::GLTexture3D* GenerateRTexture (Volume* vol,
                                     int init_x = 0,
                                     int init_y = 0,
                                     int init_z = 0,
                                     int last_x = 0,
                                     int last_y = 0,
                                     int last_z = 0);

  gl::GLTexture3D* GenerateRGBATexture (Volume* vol, TransferFunction* TF1D);

  gl::GLTexture3D* GenerateGradientTexture (Volume* vol, 
                                            int gradient_sample_size = 1,
                                            int filter_nxnxn = 0,
                                            bool normalized_gradient = true,
                                            int init_x = -1,
                                            int init_y = -1,
                                            int init_z = -1,
                                            int last_x = -1,
                                            int last_y = -1,
                                            int last_z = -1);

/*
program

uses pure2d, keyset

procedure Swap(byref a :integer, byref b :integer)
var
    c   :integer
begin
    c := a
    a := b
    b := c
end

var
    x, x0, x1, delta_x, step_x  :integer
    y, y0, y1, delta_y, step_y  :integer
    z, z0, z1, delta_z, step_z  :integer
    swap_xy, swap_xz            :boolean
    drift_xy, drift_xz          :integer
    cx, cy, cz                  :integer 

begin

    //start and end points (change these values)
    x0 := 0     x1 := -2
    y0 := 0     y1 := 5
    z0 := 0     z1 := -10
    
    //'steep' xy Line, make longest delta x plane  
    swap_xy := Abs(y1 - y0) > Abs(x1 - x0)
    if swap_xy
        Swap(x0, y0)
        Swap(x1, y1)
    endif
                
    //do same for xz
    swap_xz := Abs(z1 - z0) > Abs(x1 - x0)  
    if swap_xz
        Swap(x0, z0)
        Swap(x1, z1)
    endif
    
    //delta is Length in each plane
    delta_x := Abs(x1 - x0)
    delta_y := Abs(y1 - y0)
    delta_z := Abs(z1 - z0)
    
    //drift controls when to step in 'shallow' planes
    //starting value keeps Line centred
    drift_xy  := (delta_x / 2)
    drift_xz  := (delta_x / 2)
    
    //direction of line
    step_x := 1;  if (x0 > x1) then  step_x := -1
    step_y := 1;  if (y0 > y1) then  step_y := -1
    step_z := 1;  if (z0 > z1) then  step_z := -1
    
    //starting point
    y := y0
    z := z0
    
    //step through longest delta (which we have swapped to x)
    for x = x0 to x1 step step_x
        
        //copy position
        cx := x;    cy := y;    cz := z

        //unswap (in reverse)
        if swap_xz then Swap(cx, cz)
        if swap_xy then Swap(cx, cy)
        
        //passes through this point
        debugmsg(":" + cx + ", " + cy + ", " + cz)
        
        //update progress in other planes
        drift_xy = drift_xy - delta_y
        drift_xz = drift_xz - delta_z

        //step in y plane
        if drift_xy < 0 
            y = y + step_y
            drift_xy = drift_xy + delta_x
        endif  
        
        //same in z
        if drift_xz < 0 
            z = z + step_z
            drift_xz = drift_xz + delta_x
        endif
    next
end
*/
}

#endif
#!/usr/bin/env python
# coding=utf-8

#This script is used to calculate some parameters of interest of double-sided layer
#After calculating, you can import those parameters to tkLayout configuration to build double-sided layer geometry
# python parameter_calculation.py -h for help

import numpy as np
import math
import sys,getopt

# definition of some units
rad = 1.0
deg = math.pi/180 * rad
mm = 1.0
um = 0.001 * mm
cm = 10.0 * mm

#flag for debugging
debugflag = False

def main(argv):
    R = 60.0*mm     # layer radius
    modulewidth = 12.8*mm # width of module
    thickness = 2*mm # module thickness
    deadlength = 2*mm # length of dead area
    wireheight = 1*mm # wire height for wire bounding

    try:
        opts,args = getopt.getopt(argv,"-h-d-r:-w:-t:-l:-H:",['help','debug','radius=','width=','thickness=','deadlength=','wireheight='])
    except getopt.GetoptError:
        print '-r <layer radius> -w <module width> -t <module thickness> -l <dead area length> -H <wire height>'
        sys.exit(2)
    
    for opt,arg in opts:
        if opt in ('-h',"--help"):
            print '-r <layer radius> -w <module width> -t <module thickness> -l <dead area length> -H <wire height>'
            sys.exit()
        elif opt in ('-d','--debug'):
            global debugflag
            debugflag = True
        elif opt in ('-r','--radius'):
            R = float(arg)*mm
        elif opt in ('-w',"--width"):
            modulewidth = float(arg)*mm
        elif opt in ('-t','--thickness'):
            thickness = float(arg)*mm
        elif opt in ('-l','--deadlength'):
            deadlength = float(arg)*mm
        elif opt in ('-H','--wireheight'):
            wireheight = float(arg)*mm

    print "layer radius = %.2f\nmodule width = %.2f\nmodule thickness = %.2f\ndead length = %.2f\nwire height = %.2f\n" %(R,modulewidth,thickness,deadlength,wireheight)

    calculation(R,modulewidth,thickness,deadlength,wireheight,debugflag)

def calculation(R,modulewidth,thickness,deadlength,wireheight,debugflag):
    P = np.mat([[0],[R]])   # coordinate of module ceter
    A0 = np.mat([[modulewidth/2],[thickness/2]])
    B0 = -1 * A0
    C0 = np.mat([[modulewidth/2 + deadlength],[-1*thickness/2]])
    D0 = np.mat([[-1*modulewidth/2],[thickness/2]])
    E0 = -1 * D0
    R_10 = np.mat([[0.0],[-1*thickness/2]])
    R_20 = -1 * R_10
    if debugflag:
        print "####################debug message####################"
        print "P = "
        print P
        print "A0 ="
        print A0
        print "B0 ="
        print B0
        print "C0 ="
        print C0
        print "D0 ="
        print D0
        print "####################debug message####################"

    startRods = math.ceil(math.pi/math.asin(modulewidth/(2*R)))
    endRods = math.ceil(math.pi/math.asin(thickness/(2*R)))
    
    if debugflag:
        print "####################debug message####################"
        print "startRods = %.1f\nendRods = %.1f" %(startRods,endRods)
        print "####################debug message####################"
    
    numRods = startRods
    anglestep = 1.0*deg
    maxstep = 45
    while (numRods <= endRods):
        theta = (2*math.pi)/numRods
        # if debugflag:
            # print "####################debug message####################"
            # print "theta=%f rad" %(theta)
            # print "####################debug message####################"
             
        # module rotation matrix
        T = np.mat([[math.cos(theta),-1*math.sin(theta)],[math.sin(theta),math.cos(theta)]])
        for steps in range(1,maxstep):
            skewAngle = steps * anglestep
            # rotation matrix for skewing module
            Ms = np.mat([[math.cos(skewAngle),-1*math.sin(skewAngle)],[math.sin(skewAngle),math.cos(skewAngle)]])
            A = Ms * A0 + P
            B = Ms * B0 + P
            C = Ms * C0 + P
            D = Ms * D0 + P
            E = Ms * E0 + P
            Aprime = T * A
            Cprime = T * C
            Bprime = T * B
            Dprime = T * D
            Eprime = T * E
            # if debugflag:
                # print "####################debug message####################"
                # print "B="
                # print B
                # print ""
                # print "Aprime="
                # print Aprime
                # print ""
                # print "####################debug message####################"
            alpha1 = math.atan(B[1,0]/B[0,0])
            alpha2 = math.atan(Aprime[1,0]/Aprime[0,0])
            alpha3 = math.atan(D[1,0]/D[0,0])
            alpha4 = math.atan(Eprime[1,0]/Eprime[0,0])


            a1 = D[1,0] - A[1,0]
            b1 = A[0,0] - D[0,0]
            c1 = -1 * (a1*A[0,0] + b1*A[1,0])
            d1 = abs((a1*Cprime[0,0] + b1*Cprime[1,0] + c1)/math.sqrt(a1*a1 + b1*b1))
            
            a2 = C[1,0] - B[1,0]
            b2 = B[0,0] - C[0,0]
            c2 = -1 * (a2*B[0,0] + b2*B[1,0])
            d2 = abs((a2*Cprime[0,0] + b2*Cprime[1,0] + c2)/math.sqrt(a2*a2 + b2*b2))

            a3 = Dprime[1,0] - Aprime[1,0]
            b3 = Aprime[0,0] - Dprime[0,0]
            c3 = -1 * (a3*Aprime[0,0] + b3*Aprime[1,0])
            d3 = abs((a3*D[0,0] + b3*D[1,0] + c3)/math.sqrt(a3*a3 + b3*b3))

            a4 = Bprime[1,0] - Cprime[1,0]
            b4 = Cprime[0,0] - Bprime[0,0]
            c4 = -1 * (a4*Cprime[0,0] + b4*Cprime[1,0])
            d4 = abs((a4*D[0,0] + b4*D[1,0] + c4)/math.sqrt(a4*a4 + b4*b4))


           # if debugflag:
                # print "####################debug message####################"
                # print "alhpa1=%f; alpha2=%f; a=%f; b=%f; c=%f" %(alpha1,alpha2,a,b,c)
                # print "####################debug message####################"

            d = thickness + 0.1*mm
            if (alpha1 > alpha2 and alpha1 > alpha4 and alpha3 > alpha2 and alpha3 > alpha4 and alpha1 < 0 and alpha2 < 0 \
                and d1 > wireheight and d1 < d2 and d2 > d and d3 + d4 > d):
                R1 = Ms * R_10 + P
                R2 = Ms * R_20 + P
                r1 = math.sqrt(R1[0,0]*R1[0,0] + R1[1,0]*R1[1,0])
                layerrotation1 = math.atan(R1[1,0]/R1[0,0]) - math.pi/2
                skewAngle1 = skewAngle - math.atan(-1*R1[0,0]/R1[1,0])
                r2 = math.sqrt(R2[0,0]*R2[0,0] + R2[1,0]*R2[1,0])
                layerrotation2 = math.atan(R2[1,0]/R2[0,0]) + math.pi/2
                skewAngle2 = skewAngle - math.atan(-1*R2[0,0]/R2[1,0])
                if debugflag:
                    print "####################debug message####################"
                    print "B="
                    print B
                    print "D="
                    print D
                    print "Aprime="
                    print Aprime
                    print "####################debug message####################"
                print "Following parameters satisfy the requirement:"
                print "numRods = %.2f;\nr1 = %f mm; skewAngle1 = %f rad; layerrotation1 = %f rad;\nr2 = %f mm; skewAngle2 = %f rad; layerrotation2 = %f rad;\nskewAngle for first ladder:%f" \
                        %(numRods,r1,skewAngle1,layerrotation1,r2,skewAngle2,layerrotation2,skewAngle)
                if (alpha1 > alpha3):
                    if (alpha2 > alpha4):
                        print "first overlap region: %f, %f, %f, %f" %(alpha4+math.pi/2,alpha2+math.pi/2,alpha3+math.pi/2,alpha1+math.pi/2)
                        print "the length of overlap in phi are: %f, %f\n" %(alpha1-alpha4,alpha3-alpha2)
                    else:
                        print "first overlap region: %f, %f, %f, %f" %(alpha2+math.pi/2,alpha4+math.pi/2,alpha3+math.pi/2,alpha1+math.pi/2)
                        print "the length of overlap in phi are: %f, %f\n" %(alpha1-alpha2,alpha3-alpha4)

                if (alpha1 < alpha3):
                    if (alpha2 > alpha4):
                        print "first overlap region: %f, %f, %f, %f" %(alpha4+math.pi/2,alpha2+math.pi/2,alpha1+math.pi/2,alpha3+math.pi/2)
                        print "the length of overlap in phi are: %f, %f\n" %(alpha3-alpha4,alpha1-alpha2)
                    else:
                        print "first overlap region: %f, %f, %f, %f" %(alpha2+math.pi/2,alpha4+math.pi/2,alpha1+math.pi/2,alpha3+math.pi/2)
                        print "the length of overlap in phi are: %f, %f\n" %(alpha3-alpha2,alpha1-alpha4)

                break

        numRods = numRods + 1

if __name__ == "__main__":
    main(sys.argv[1:])










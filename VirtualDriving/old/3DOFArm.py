import math
import array

while True:

    #Input Values, to be deleted
    xd=input("X:")
    yd=input("Y:")
    xd=float(xd)
    yd=float(yd)
    #xd=recieve value
    #yd=recieve value

    #Define Manipulator Parameter
    OrientationArray=[0,math.pi/2,math.pi/4,math.pi/6,-math.pi/2,-math.pi/4,-math.pi/6,-math.pi,0.1,0.7]
    l1=14.5
    l2=18.5
    l3=18
    flag=0
    col=0



    #Find Angles and Corresponding Orientation
    while flag==0:
        Orientation=OrientationArray[col]
        x=xd-(l3*math.cos(Orientation))
        y=yd-(l3*math.sin(Orientation))
        D=((math.pow(x,2))+(math.pow(y,2))-(math.pow(l1,2))-(math.pow(l2,2)))/(2*l1*l2)
        if(isinstance(D, float) & (pow(D,2)<=1)):
            Angle2=math.atan2(-math.sqrt(1-(math.pow(D,2))),D)
            Angle1=math.atan2(y,x)-math.atan2(l2*math.sin(Angle2),l1+(l2*math.cos(Angle2)))
            Angle3=Orientation-Angle1-Angle2
            if( (Angle1>math.pi) | (Angle1<0) | (Angle2>0) | (Angle2<-math.pi) | (Angle3>0) | (Angle3<-math.pi) ):
                flag=0
            else:
                flag=1
        col=col+1


    #Check Values using Forward Kinematics
    dx=18*math.cos(Angle1 + Angle2 + Angle3) + (37*math.cos(Angle1 + Angle2))/2 + (29*math.cos(Angle1))/2
    dy=18*math.sin(Angle1 + Angle2 + Angle3) + (37*math.sin(Angle1 + Angle2))/2 + (29*math.sin(Angle1))/2

    #Angles to pulses
    Pulse1=int(round(((185.0/18)*math.degrees(Angle1))+550))
    Pulse2=int(round(((185.0/18)*abs(math.degrees(Angle2)))+550))
    Pulse3=int(round(((185.0/18)*abs(math.degrees(Angle3)))+550))

    #Printing Values
    print "Angle1=",math.degrees(Angle1)
    print "Angle2=",math.degrees(Angle2)
    print "Angle3=",math.degrees(Angle3)
    print "X=",dx
    print "Y=",dy
    print "Orientation=",math.degrees(Orientation)
    print "Pulse1=",Pulse1
    print "Pulse2=",Pulse2
    print "Pulse3=",Pulse3


    

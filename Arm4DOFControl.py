import math
import array
import numpy
import time
import serial
ser=serial.Serial('/dev/ttyAMA0',9600)
ser.write('a')#Attach Servos



#Input Values
x=float(input("X:"))
y=float(input("Y:"))
z=float(input("Z:"))

#Define Manipulator Parameter
OrientationArray1=[0,math.pi/2,math.pi/4,math.pi/6,-math.pi/2,-math.pi/4,-math.pi/6,-math.pi/8,math.pi/8,-math.pi,math.pi,0.1,0.7]
OrientationArray2=numpy.linspace(-2*math.pi,2*math.pi,num=99999)
OrientationArray=numpy.concatenate((OrientationArray1,OrientationArray2),axis=0)
l1=14.5
l2=18.5
l3=18

#Initialise Variables
Angle1=math.pi;
Angle2=2*math.pi;
Angle3=math.pi;
Angle4=math.pi;
i=0;

#Find Angles and Corresponding Orientation
while (Angle1<(-math.pi/2) or Angle1>(math.pi/2) or Angle3<-math.pi or Angle3>0 or Angle4<-math.pi or Angle4>0 or Angle2<0 or Angle2>math.pi or round(x)!=round(dx) or round(y)!=round(dy) or round(z)!=round(dz)):

    Orientation=OrientationArray[i]
    i=i+1;

    Angle1=math.atan2(y,x)
    if(Angle1<(-math.pi/2) or Angle1>(math.pi/2)):
        Angle1=Angle1-math.pi

    A=math.sqrt(math.pow(x,2)+math.pow(y,2))-(l3*math.cos(Orientation))
    B=z-(l3*math.sin(Orientation))
    D=(math.pow(A,2)+math.pow(B,2)-math.pow(l1,2)-math.pow(l2,2))/(2*l1*l2) 
    if(isinstance(D, float) & (pow(D,2)<=1)):  
        Angle3=math.atan2(-math.sqrt(1-math.pow(D,2)),D)
        if(Angle3<-math.pi or Angle3>0):
          Angle3=Angle3-math.pi
   
        Num=l2*math.sin(Angle3)
        Den=l1+(l2*math.cos(Angle3))
        try: 
            Angle2=math.atan2(B,A)-math.atan2(Num,Den)
        except:
            Angle2=-math.pi

        Angle4=Orientation-Angle2-Angle3
    else:
        Angle2=-math.pi
        Angle3=math.pi
        Angle4=math.pi
    #Check Values using Forward Kinematics
    dx=(math.cos(Angle1)*(36*math.cos(Angle2 + Angle3 + Angle4) + 37*math.cos(Angle2 + Angle3) + 29*math.cos(Angle2)))/2
    dy=(math.sin(Angle1)*(36*math.cos(Angle2 + Angle3 + Angle4) + 37*math.cos(Angle2 + Angle3) + 29*math.cos(Angle2)))/2
    dz=18*math.sin(Angle2 + Angle3 + Angle4) + (37*math.sin(Angle2 + Angle3))/2 + (29*math.sin(Angle2))/2




#Angles to pulses
Angle1=Angle1+(math.pi/2)
Pulse1=int(round(((185.0/18)*abs(math.degrees(Angle1)))+550))
Pulse2=int(round(((185.0/18)*abs(math.degrees(Angle2)))+550))
Pulse3=int(round(((185.0/18)*abs(math.degrees(Angle3)))+550))
Pulse4=int(round(((185.0/18)*abs(math.degrees(Angle4)))+550))

#Validation
if(round(x)==round(dx) and round(y)==round(dy) and round(z)==round(dz)):
    print "CHECKED"
else:
    print "ERROR"


#Printing Values
print "Xi=",x
print "Yi=",y
print "Zi=",z
print "Angle1=",math.degrees(Angle1)
print "Angle2=",math.degrees(Angle2)
print "Angle3=",math.degrees(Angle3)
print "Angle4=",math.degrees(Angle4)
print "Orientation=",math.degrees(Orientation)
print "Xo=",dx
print "Yo=",dy
print "Zo=",dz
print "Pulse1=",Pulse1
print "Pulse2=",Pulse2
print "Pulse3=",Pulse3
print "Pulse4=",Pulse4

#Send Them Serially
ser.write('5')#Open Arm
time.sleep(0.2)
Pulse1=str(Pulse1+1000)
Pulse2=str(Pulse2+1000)
Pulse3=str(Pulse3+1000)
Pulse4=str(Pulse4+1000)
ser.write('1')
ser.write(Pulse1)
ser.write('2')
ser.write(Pulse2)
ser.write('3')
ser.write(Pulse3)
ser.write('4')
ser.write(Pulse4)
ser.write('s')#Actuate
ser.write('c') #Close Arm


#Return Arm Back to Initial Position
Send=input("Send Back:")
ser.write('1')
ser.write('2475')
ser.write('2')
ser.write('2783')
ser.write('3')
ser.write('2269')
ser.write('4')
ser.write('2783')
ser.write('s')#Actuate
ser.write('d')#Detach Servos






















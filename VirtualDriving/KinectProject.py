def sendCommand(x):
        import serial
        import math
        import array
        import numpy
        ser=serial.Serial('/dev/ttyAMA0',9600)

#Recieving Data
        #Steering Wheel 
        Steering=x[0] 
        xR=x[3] 
        yR=x[4]
        xL=x[5] 
        yL=x[6] 
        #Manipulator 
        ScalingX=15
        ScalingZ=20
        xd=(x[1]*100.0)-ScalingX
        yd=x[7] *100.0
        zd=x[2]*100.0+ScalingZ
        #Hand State
        HandState=x[7]

#Defining Parameters
        #Driving
        SpeedRange=10*xd
        BackwardSpeed=200
        MaxSteeringAngle=90
        SpeedR=0
        SpeedL=0
        #Manipulator
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

#################################
####### #STEERING WHEEL #########
#################################
        if HandState== 1:
                print "STEERING"
                #Computing Angles
                Num=xR-xL
                Den=math.sqrt(math.pow((xR-xL),2)+math.pow((yR-yL),2))
                Angle=math.acos((Num*1.0)/Den)
                Angle=math.degrees(Angle)
                print "Angle:" , Angle

                #Processing Direction
                if Steering== 0:        #Stop
                        SpeedR=0
                        SpeedL=0
                        print "Stop"
                elif Steering == 4:     #Backward
                        SpeedR=-BackwardSpeed
                        SpeedL=-BackwardSpeed
                        print "Backward"
                elif Steering== 1:      #Forward  
                        SpeedR=round(SpeedRange)
                        SpeedL=round(SpeedRange)
                        print "Forward"
                elif Steering== 3:      #Right
                        SpeedR=round(SpeedRange)
                        SpeedL=round(((-200.0/MaxSteeringAngle)*Angle)+200.0)
                        print "Right"
                elif Steering== 2:      #Left
                        SpeedL=round(SpeedRange)
                        SpeedR=round(((-200.0/MaxSteeringAngle)*Angle)+200.0)
                        print "Left"


                #Remove any negative value
                if (SpeedR<0 and SpeedL<0):
                        SpeedR=str(-SpeedR+100)
                        SpeedL=str(-SpeedL+100)
                        ser.write('r')
                        ser.write(SpeedR)
                        ser.write('l')
                        ser.write(SpeedL)
                        ser.write('b')
                else:
                        
                        if (SpeedR<0):
                                SpeedR=0
                        if (SpeedL<0):
                                SpeedL=0
                                


                        print "SpeedR:" ,  SpeedR
                        print "SpeedL:" , SpeedL


                        #Right speeds to serial 
                        SpeedR=str(SpeedR+100)
                        SpeedL=str(SpeedL+100)
                        ser.write('r')
                        ser.write(SpeedR)
                        ser.write('l')
                        ser.write(SpeedL)
                        ser.write('w')



############################
####### #4 DOF ARM #########
############################
        
        elif HandState== 0:
            #Set a maximum limit for xd and yd in order to avoid any error
                if(xd<0):
                        print "OvrLimits"
                        xd=0
                if(xd>30):
                        print "OvrLimits"
                        xd=30
                if(yd<-28):
                        print "OvrLimits"
                        yd=-28
                if(yd>28):
                        print "OvrLimits"
                        yd=28
                if(zd<0):
                        print "OvrLimits"
                        zd=0
                if(zd>30):
                        print "OvrLimits"
                        zd=30
                        
                

            #Find Angles and Corresponding Orientation
                while (Angle1<(-math.pi/2) or Angle1>(math.pi/2) or Angle3<-math.pi or Angle3>0 or Angle4<-math.pi or Angle4>0 or Angle2<0 or Angle2>math.pi or round(xd)!=round(dx) or round(yd)!=round(dy) or round(zd)!=round(dz)):

                    Orientation=OrientationArray[i]
                    i=i+1;

                    Angle1=math.atan2(yd,xd)
                    if(Angle1<(-math.pi/2) or Angle1>(math.pi/2)):
                        Angle1=Angle1-math.pi

                    A=math.sqrt(math.pow(xd,2)+math.pow(yd,2))-(l3*math.cos(Orientation))
                    B=zd-(l3*math.sin(Orientation))
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
                if(round(xd)==round(dx) and round(yd)==round(dy) and round(zd)==round(dz)):
                    print "CHECKED"
                else:
                    print "ERROR"


            #Printing Values
                print "Xi=",xd
                print "Yi=",yd
                print "Zi=",zd
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
              
        

#Send Angles to Serial
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
                
                if(HandState== 0):
                        ser.write('o')#Open Arm
                else:
                        ser.write('c') #Close Arm
                        
                


    









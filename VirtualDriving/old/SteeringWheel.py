import math

#Initialising Speed
MaxSpeed=200
MaxSteeringAngle=45
SpeedR=0
SpeedL=0

while True:
    #Accepting x and y Coordinated
    xR=float(input("xR:"))
    yR=float(input("yR:"))
    xL=float(input("xL:"))
    yL=float(input("yL:"))

    #Computing Angles
    Num=xR-xL
    Den=math.sqrt(math.pow((xR-xL),2)+math.pow((yR-yL),2))
    Angle=math.acos((Num*1.0)/Den)
    Angle=math.degrees(Angle)
    print "Angle:" , Angle


#Setting Respective Speeds

    #Forward    
    if (abs(yR)==abs(yL)):
        SpeedR=MaxSpeed
        SpeedL=MaxSpeed

    #Right    
    if (yR>yL):
        SpeedR=MaxSpeed
        SpeedL=round(((-200.0/MaxSteeringAngle)*Angle)+200.0)

    #Left    
    if (yL>yR):
        SpeedL=MaxSpeed
        SpeedR=round(((-200.0/MaxSteeringAngle)*Angle)+200.0)


    print "SpeedR" ,  SpeedR
    print "SpeedL:" , SpeedL
    

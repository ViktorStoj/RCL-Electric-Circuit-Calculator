/*------------------------------------------------------------------
File: Main.c
Name of program: RCL Electric Circuit Calculator

EE_Data.txt:
    Holds user data obtained from RCL_Electric_Circuit_Calculator.c
Description:
        The program reads EE_Data.txt to receive any past data. The user is prompted to use previous data or input their own.

        If user uses previous data, selected data with be used to plot function q(t) (Charge over Time).

        If user chooses to input their own data, they input: L, C, the battery voltage, the dissipation time td,
    and the percent of original charge pc to reach within dissipation time into the program when prompted.

        The program, using the bisection method, calculates the root R and displays it to the user. The
    program will plot q(t) (Charge over Time) with the newly acquired data.

        The user will be prompted to store data into the file EE_Data.txt. If a data slot is available,
    they may save it there. Otherwise, they will be prompted to replace an existing data slot.

---------------------------------------------------------------------*/
#include <stdio.h>
#include "plplot.h"
//#include "gng1106plplot.h"
#include <gng1106plplot.h>  // provides definitions for using PLplot library
#include <math.h>
#include <float.h>

// Some definitions
#define N 100   // Number of points used for plotting
#define XINC 0.1         // for incrementing x when computing points
#define X_IX 0           // Row index for storing x values
#define FX_IX 1          // Row index for storing f(x) values
#define NAMEFILE "EE_Data.txt"
#define EPSILON 1e-10

///Structure
typedef struct data{
    double L ;
    double C ;
    double V ;
    double td ;
    double pc ;
    double R  ;
    int full ;
} DATA;


///Functions prototypes

///User Functions
void getUserData(DATA [], DATA*);
void SaveData(DATA[],DATA*);
double overZero(char []);
///Calculation Functions
double qfunc(DATA, double);
double Rfunc(DATA, double);
double findRoot(double, double, DATA);
double findUpper(DATA);
int validate(DATA);
///Plotting Functions
void plotFunc(double, double, DATA);
void plot(int, double *, double *);
double getMin(double *array, int n);
double getMax(double *array, int n);
///File Functions
void getFileData(DATA []);
void writeFileData(DATA []);

/*----------------------------------------------------------
Function: main
Parameters:
    None
Returns:
    None
Description: This function starts off the program. It will call on other
functions and will dictate the order of when a function is used. When
this function ends, the program will end.
----------------------------------------------------------------*/
int main(){
    DATA fileData[5];
    DATA selectedData;
    getFileData(fileData);
    //writeFileData(fileData);
    int flag = 1;
    do{
        flag = 0;
        getUserData(fileData, &selectedData);
        printf("Data chosen:\n");
        printf("L: %lf\n", selectedData.L);
        printf("C: %lf\n", selectedData.C);
        printf("V: %lf\n", selectedData.V);
        printf("td: %lf\n", selectedData.td);
        printf("pc: %lf\n", selectedData.pc);
        //printf("full: %d\n", selectedData.full);

        if(!validate(selectedData)){
            flag = 1;
            printf("\nValues given will not yeild an R value\n");
            fflush(stdin);
        }

    }while(flag);
    int fileOrnot = 1;
    if(selectedData.full == 0){
        fileOrnot = 0;
            //printf("\n%lf", findUpper(selectedData));
        selectedData.R = findRoot(0, findUpper(selectedData), selectedData);
        selectedData.full = 1;
    }
    printf("R: %lf\n", selectedData.R);

    plotFunc(0, selectedData.td, selectedData);
    if(fileOrnot == 0){
        SaveData( fileData, &selectedData);
    }
    return 0;
}
/*----------------------------------------------------------
Function: getFileData
Parameters:
    data[]: Structure DATA keeps values the user has inputted. It is an array that carries up to 5 data sets.
Returns
    None
Description: This function opens file UserData.bin and extracts any past data from it.
It will use fread to transfer all data into the array arr. Each index of data is
representative of a data set.
----------------------------------------------------------------*/
void getFileData(DATA data[]){
    FILE *ptr = fopen(NAMEFILE, "r");
    for(int ix = 0; ix<5 || !feof(ptr); ix++){
        fscanf(ptr, "%lf",&data[ix].L);
        fscanf(ptr, "%lf",&data[ix].C);
        fscanf(ptr, "%lf",&data[ix].V);
        fscanf(ptr, "%lf",&data[ix].td);
        fscanf(ptr, "%lf",&data[ix].pc);
        fscanf(ptr, "%lf",&data[ix].R);
        fscanf(ptr, "%d ",&data[ix].full);
    }
    fclose(ptr);

}
/*----------------------------------------------------------
Function: writeFileData
Parameters:
    data[]: Structure DATA keeps values the user has inputted. It is an array that carries up to 5 data sets.
Returns:
    None
Description: This function opens file UserData.bin and rewrite the file with
the values in arr[]. It will use fwrite to transfer all data into the file. Each
index of data is representative of a data set.
----------------------------------------------------------------*/
void writeFileData(DATA data[]){
    FILE *ptr = fopen(NAMEFILE, "w");

    for(int ix = 0; ix<5; ix++){
        fprintf(ptr, "%lf ",data[ix].L);
        fprintf(ptr, "%lf ",data[ix].C);
        fprintf(ptr, "%lf ",data[ix].V);
        fprintf(ptr, "%lf ",data[ix].td);
        fprintf(ptr, "%lf ",data[ix].pc);
        fprintf(ptr, "%lf ",data[ix].R);
        fprintf(ptr, "%d \n",data[ix].full);
    }
    fclose(ptr);
}
/*----------------------------------------------------------
Function: SaveData
Parameters:
    DATA file: Keeps values obtained through the file UserData.bin. It is an array that carries up to 5 data sets.
    DATA selected: Contains values that the user chose.
Returns:
    None
Description: The user is presented with all previous user data contained in
DATA file. The user will make a choice to save data they have selected into
the DATA file. If DATA file has less than 5 data sets, the user can save the
data into a free slot. If the data has 5 data sets, the user will be prompted
to replace a data set of their choosing.
----------------------------------------------------------------*/
void SaveData(DATA fData[], DATA *sData){
    int flag = 1;
    int slot;
    for(int ix = 4;ix>=0;ix--){
        if(fData[ix].full == 0){
            flag =  0;
            slot = ix;
        }
    }
    if(flag){
        char input;
        flag = 1;
        do{
            flag = 0;
            printf("Would you like to overwrite a data slot (Y/N)? ");
            fflush(stdin);
            scanf("%c", &input);
            fflush(stdin);
            if(input != 'Y' && input != 'N'){
                printf("Please enter Y (for yes) or N (for no) ");
                flag = 1;
            }
        }
        while (flag);
        if(input == 'Y'){
                flag =1;
            do{
                flag = 0;
                printf("Please chose a data slot (0-4): ");
                scanf("%d", &slot);
                if(slot<0 || slot>4){
                    printf("Please input a value between 0-4\n");
                    flag = 1;
                }
            }
            while (flag);
            fData[slot] = *sData;
            printf("\nData has been saved to slot %d", slot);
            writeFileData(fData);
        }
        else{
            printf("\nData has not been saved");
        }
    }
    else{
        fData[slot] = *sData;
        printf("\nData has been saved to slot %d", slot);
        writeFileData(fData);
    }

}
/*----------------------------------------------------------
Function: getUserData
Parameters:
    DATA file: Keeps values obtained through the file UserData.bin. It is an array that carries up to 5 data sets.
    DATA selected: Contains values that the user chose.
    DATA data: Contains all user data
Returns:
    None
Description: The user is presented with all previous user data contained
in DATA file. The user will make a choice to select data from the presented
data or input it in themselves. Selected data will be put into DATA selected
for future use in the program.
----------------------------------------------------------------*/
void getUserData(DATA fData[], DATA *sData){
    printf("\nData obtained from %s:\n", NAMEFILE);
    for(int ix = 0; ix<5; ix++){
        printf("\nSLOT %d: \n", ix);
        if(fData[ix].full == 1){
            printf("L: %lf\n", fData[ix].L);
            printf("C: %lf\n", fData[ix].C);
            printf("V: %lf\n", fData[ix].V);
            printf("td: %lf\n", fData[ix].td);
            printf("pc: %lf\n", fData[ix].pc);
            printf("R: %lf\n", fData[ix].R);
        }
        else{
            printf("N/A\n");
        }
    }
    char input;
    int flag = 1;
    do{
        flag = 0;
        printf("Please chose to select data from file (f) or input your own data (w): ");
        fflush(stdin);
        scanf("%c", &input);
        fflush(stdin);
        if(input != 'f' && input != 'w'){
            printf("Please enter f (for file) or w (for write)\n");
            flag = 1;
        }
    }
    while (flag);

    if(input == 'f'){
        int slot =0;
        flag =1;
        do{
            flag = 0;
            printf("Please chose a data slot (0-4): ");
            scanf("%d", &slot);
            if(slot<0 || slot>4){
                printf("Please input a value between 0-4\n");
                flag = 1;
            }
        }
        while (flag);
        *sData = fData[slot];
    }
    else{
        flag = 1;
        double temp;
        fflush(stdin);
        sData->L = overZero("L");
        fflush(stdin);
        sData->C = overZero("C");
        fflush(stdin);
        sData->V = overZero("V");
        fflush(stdin);
        sData->td = overZero("td");
        fflush(stdin);
        do{
            flag = 0;
            temp = overZero("pc");
            if(temp>1){
                flag =1;
                printf("pc cannot be greater than 1\n");
            }
        }
        while(flag);
        sData->pc = temp;

        sData->full = 0;
    }
}
/*----------------------------------------------------------
Function: overZero
Parameters:
    char var[2]: This string correlates to a specific data point from the structure.
Returns:
    The specific data point’s value given by the user
Description: This function will be used to get user inputs that are not
negative or equal to zero.
----------------------------------------------------------------*/
double overZero(char var[2]){
    int flag = 1;
    double input;
    do{
        flag = 0;
        printf("Please input a value for %s: ", var);
        scanf("%lf", &input);
        if(input<= 0){
            printf("Number must be greater than 0\n");
            flag = 1;
        }
    }
    while(flag);
    return (input);
}
/*----------------------------------------------------------
Function: findUpper
Parameters:
    parameter: Data the user has chosen.
Returns:
    Upper value of the interval used to find a root in function g(R)
Description: This function calculates the upper bound using the L and C values
determined from DATA parameter.
----------------------------------------------------------------*/
double findUpper(DATA data){
    double upper = 4*data.L;
    upper = upper/data.C;
    upper = sqrt(upper);
    return upper;
}
/*----------------------------------------------------------
Function: validate
Parameters:
    Data the user has selected
Returns:
    Returns 0 if user data will not yield an R value. Returns 1 if user data will yield an R value.
Description: This function checks to see if the values that the user inputted work
within the program.
----------------------------------------------------------------*/
int validate(DATA data){
    double R = (-2*data.L*log(data.pc))/(data.td);
    double ans = (1)/(data.L*data.C);
    //print("\n %lf", ans);
    ans = ans - pow(R/(2*data.L), 2);
    //printf("\n%lf\n", ans);
    if(ans < 0){
        return 0;
    }
    else{
        return 1;
    }
}
/*----------------------------------------------------------
Function: findRoot
Parameters:
    lower: Lowest x value of interval (0)
    upper: Lowest x value of interval (Rmax)
Returns:
    Root of a function at a given interval
Description: Returns root at a given interval of a function.
The method to find the root is the bisection method using the lower
and upper bound given.
---------------------------------------------------------------*/
double findRoot(double lower, double upper, DATA data){
    double mid;
    //int flag = 0;
    while( Rfunc(data, lower)*Rfunc(data, upper)*Rfunc(data ,lower)*Rfunc(data,upper) >= EPSILON*EPSILON){
    //        flag =1;
            Rfunc(data, lower)*Rfunc(data, upper)*Rfunc(data ,lower)*Rfunc(data,upper);
            mid = (lower + upper)/2;
            if(Rfunc(data,mid)*Rfunc(data,upper)< 0){
                    lower = mid;
            }
            if(Rfunc(data, lower)*Rfunc(data, mid)< 0){
                    upper = mid;
            }
            //printf("%lf\n", Rfunc(data, lower)*Rfunc(data, upper));
    }
    double root = (lower + upper)/2;

    return(root);

}
/*----------------------------------------------------------
Function: Rfunc
Parameters:
    t: specific time the of the charge over time function
    parameters: specific values inputted by user
Returns:
    Value of g(R) at a given R
Description: Returns value of g(R) at a given resistance using data the user
inputted previously and the time. It will use the g(R) equation.
---------------------------------------------------------------*/
double Rfunc(DATA parameters, double R){
   return (exp((-R*parameters.td)/(2*parameters.L))-parameters.pc);
}
/*----------------------------------------------------------
Function: qfunc
Parameters:
    t: specific time the of the charge over time function
    parameters: specific values inputted by user
Returns:
    Specific charge at a given time
Description: Returns the specific charge at a given time using data the user
inputted previously and the time. It will use the q(t) equation.
---------------------------------------------------------------*/
double qfunc(DATA parameters, double t){
    double q0 = parameters.V*parameters.C;
    double ans = q0*exp(-parameters.R*t/(2*parameters.L));
    ans = ans*cos(sqrt((1/(parameters.L*parameters.C))-pow(parameters.R/(2*parameters.L),2))*t);
    return ans;
}
/*----------------------------------------------------------
Function: plotFunc
Parameters:
    begin: First x value in the function to be plotted
    end: Last x value in the function to be plotted
    Data: Contains all user data
Returns:
    none
Description: Calculates the points (x and y values) on a function within the interval given
in the parameter and puts them in 2 separate arrays respectively. It sends these arrays to a
plotting function to plot the points calculated.
---------------------------------------------------------------*/
void plotFunc(double begin, double end, DATA data){
    double x[N];
    double y[N];
    double inc; // increment for incrementing x
    int ix;
    // Calculate function points
    inc = (end - begin)/N;
    x[0] = begin;
    y[0] = qfunc(data, x[0]); // Compute first point
    for(ix = 1; ix < N; ix = ix + 1)
    {
        x[ix] = x[ix -1] + inc;
        y[ix] = qfunc(data,x[ix]);
    }
    // Plot
    plot(N, x, y);

}
/*----------------------------------------------------------
Function: plot
Parameters:
    n: The number of points in the arrays, which is 100.
    xPtr: This is a pointer to the x values that signify the t (horizontal axis).
    yPtr: This is a pointer to the y values that signify the q(t)  (vertical axis).
Returns:
    None
Description:
Initialises the plot.  The following values in the referenced structure
are used to set up the plot x[0], x[n-1] - assume that x values are sequential miny,
maxy - vertical axis range (add 10% to min/max value) Sets up white background and black
for ground colors. Then plots the curve accessed using xPtr and yPtr.
---------------------------------------------------------------*/
void plot(int n, double *xPtr, double *yPtr){
    double miny, maxy;
    double range;  // range of vertical axix
        // Setup plot configuration
    plsdev("wingcc");  // Sets device to wingcc - CodeBlocks compiler
    // Initialise the plot
    plinit();
    // Configure the axis and labels
    plwidth(3);          // select the width of the pen
    // Find range for axis
    miny = getMin(yPtr, n);
    maxy = getMax(yPtr, n);
    range = maxy - miny;  // the width of the range
    maxy = maxy + 0.1*range;
    miny = miny - 0.1*range;
    plenv0(xPtr[0], xPtr[n-1], miny, maxy, 0, 1);
    plcol0(BLUE);           // Select color for labels
    pllab("t", "q(t)", "Dissipation of Charge(q) over Time(t)");
    // Plot the velocity.
    plcol0(GREEN);    // Color for plotting curve
    plline(n, xPtr, yPtr);
    // Plot the points
    plend();

}

/*----------------------------------------------------------
Function: getMin
Parameters:
    array: reference to an array with double values
    n: number of elements in the array
Returns
    min:  the minimum value found in the array
Description: Traverses the array to find its minimum value.
----------------------------------------------------------------*/
double getMin(double *array, int n)
{
    int ix;
    double min = array[0];
    for(ix = 1; ix < n; ix = ix +1)
        if(min > array[ix]) min = array[ix];
    return(min);
}

/*----------------------------------------------------------
Function: getMax
Parameters:
    array: reference to an array with double values
    n: number of elements in the array
Returns
    max:  the maximum value found in the array
Description: Traverses the array to find its maximum value.
----------------------------------------------------------------*/
double getMax(double *array, int n)
{
    int ix;
    double max = array[0];
    for(ix = 1; ix < n; ix = ix +1)
        if(max < array[ix]) max = array[ix];
    return(max);
}

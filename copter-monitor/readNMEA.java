import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Scanner;

public class readNMEA {
	public static void main(String[] args) throws IOException{
		ArrayList<String[]> data = new ArrayList<String[]>();
		File file = new File ("nmealoggps.txt");
		//this will be reading a serial input, not a text file
		//need to look into how that works with getting the data real-time (ie won't use for loops, will use while loops instead, etc.
		Scanner inputFile = new Scanner (file);
		while (inputFile.hasNext())
		{
			String str = inputFile.nextLine();
			String[] line = str.split(",");
			//put processing code here
			//have data be the already processed arraylist
			//only add lines of data that are what we want
			data.add(line);				
		}
		inputFile.close();
		FileWriter fw = new FileWriter("C:\\Users\\Student\\Documents\\Weather copter\\nmeadata.csv");
		PrintWriter pw = new PrintWriter(fw);
		ArrayList<String[]> newdata = parse(data);
		for (int i=0; i<newdata.size(); i++){
			pw.println(Arrays.asList(newdata.get(i)));
		}
		//Flush the output to the file
		pw.flush();

		//Close the Print Writer
		pw.close();

		//Close the File Writer
		fw.close();    
	}
	//make this a String[] function, not arraylist, process data line by line
	public static ArrayList<String[]> parse(ArrayList<String[]> array) {
		String time = ""; //variable that will be added to every row that goes in that timestamp
		String date = "";
		String temp = "";
		String lat = "";
		String lng = "";
		String pres = "";
		String hum = "";
		String wdir = "";
		String wspd = "";
		ArrayList<String[]> newarray = new ArrayList<String[]>();
		//String[] a = new String[9];
		//int num = 0;
		int q = 0;
		boolean start = false;
		while (start==false){
			String[] nmea = array.get(q);
			String sentence = nmea[0];
			if(sentence.equals("$GPZDA")){
				start = true;
			}
			else if (!(sentence.equals("GPZDA"))){
				q++;
			}
		}
		
		for(int i=q; i<array.size(); i++){
			String[] nmea = array.get(i);
			String sentence = nmea[0];
			if (sentence.equals("$GPZDA"))
			{
				if (nmea[1]!=time && !time.equals("")){
					String [] a = {date, time, lat, lng, pres, temp, hum, wdir, wspd};
					System.out.println(Arrays.asList(a));
					newarray.add(a);
				}
				time = nmea[1];
				date = nmea[2] + nmea[3] + nmea[4];
				//num++;
				//t = time;
				//String [] data = {date, time};

				//System.out.println(Arrays.asList(data));
			}
			//while (!(time.equals(""))){
			//String[] a = new String[]();
			//				if (sentence.equals("$GPZDA"))
			//				{
			//					time = nmea[1];
			//					date = nmea[2] + nmea[3] + nmea[4];
			//					String [] data = {date, time, "", "", "", "", "", "", ""}; 
			//					System.out.println(Arrays.asList(a));
			//					newarray.add(a);
			//					num++;
			//				}
			if (sentence.equals("$GPGGA"))
			{
				lat = nmea[2] + nmea[3];
				lng = nmea[4] + nmea[5];
				//String[] data = {lat, lng}; 
				//				System.out.println(Arrays.asList(data));
				//				a.add(num, data);
				//num++;
			}
			if (sentence.equals("$WIMDA"))
			{
				//reads wind direction measurements and puts in an array by pressure, temperature, humidity, wind direction, and wind speed (in that order)
				pres = nmea[3];
				temp = nmea[5];
				hum = nmea[9];
				wdir = nmea[15];
				wspd = nmea[19];
				//num++;
			}
//			
//			if(num%3 ==0){
//				String [] a = {date, time, lat, lng, pres, temp, hum, wdir, wspd};
//				System.out.println(Arrays.asList(a));
//				newarray.add(a);
//			}
		}
		return newarray;
	}
}

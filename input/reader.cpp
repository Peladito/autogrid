#include <iostream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <fstream>

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------Auxiliar para carlos's potential---------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Se le pasa una linea con data del *.dat de los WaterSites y devuelve el dato que se encuentre en la columna con indice "posicion"
//El parseo reconoce como token de ceparacion al TAB ("\t")
std::string gsfp(std::string s, int posicion, std::string delimiter){

	size_t pos = 0;

	while ((pos = s.find(delimiter)) != std::string::npos && posicion>0) {
		
		s.erase(0, pos + delimiter.length());
		posicion--;
	}
	if (pos == std::string::npos || posicion<=0)
	{
		return "";
	}
	return s.substr(0, pos).c_str();
}
double get_data_from_pos(std::string s, int posicion){
	printf("a ver: %s\n", gsfp(s, posicion, "\t").c_str());
	return atof(gsfp(s, posicion, "\t").c_str());
}

	//La siguiente funcion recive el argc y argv del main y recupera el parámetro -w el cual no lo pude recuperar en una global desde setflags (work around)

const char* get_w_parameter( const int argc,const char **argv){
	for (int i = 1; i < argc; i++) { 
					 //if (i + 1 != argc) 
		if (std::string(argv[i]) == "-w") {
			return argv[i + 1];
		} 
	}

	return NULL;
}




//Estructura auxiliar: Aca se va a guardar los limites del cubo que comprende el area alrrededor del WS en donde se quiere modificar ls valores de la grilla
//[FIXME][ATENCION]: Las variables x_max y y_min no se utilizan, pero por alguna razon de ser borradas generan un segmentation fault, asi como el agregar mas variables
//Una posible mejora es dejar de considerar un cubo y pasar a una esfer (slo guardariamos el centro de la esfera y su energia)
typedef struct cubo {
	double x_min, x_max, y_min, y_max, z_min, z_max, xmax, ymin, zc, en;
	std::string type;

		//parseo el tipo que va a estar guardado con delimitadores '_', un dolor de cabeza...pero bueno, el formato es asi
		//[TO TEST]
	bool isType(std::string t){
		std::size_t found = type.find(t);
		return found!=std::string::npos;
	}
} Cubo;


//Se defina la estructura principal: en ella se van a guardar: el radio de los cubos, la cantidad de WS, la resolucion de la grilla y el centro de la grilla en coordenadas de la grilla
typedef struct gridTransformer {
	int cantWs;
	int last_cubo;
	double resolucion;
	cubo* listWs;
	double centro_grilla_x;
	double centro_grilla_z;
	double centro_grilla_y;

	gridTransformer(){
		last_cubo = 0;
	}

		/*
		Constructor parametrizado

		cant = Cantidad de watersites a crear
		resol = Resolucion de la grilla donde operar
		rad = radio del water site
		cx = centro x de la grilla
		cy = centro y de la grilla
		cz = centro z de la grilla 
		filename= ruta al .dat
		*/
	gridTransformer( double cx, double cy, double cz, double resol, const char* filename): last_cubo(0), resolucion(resol){
		std::string line;
		printf("%s\n", filename );
		std::ifstream myfile(filename);
		int header = 8;

			// radio = 2; //En distancia grilla
		printf("%s\n","Creando GridTransformer:");
		addMetadata(filename, cx, cy, cz);

		if (myfile.is_open())
		{
			std::string word;
			int col = 0;
			double x,y,z,en,r90;
			while (myfile >> word )
			{
				if (header>=0){
					printf("%s| ",word.c_str());
					header--;
				}else{

					
					switch (col){
						case 0:
							//#SS
						break;
						case 1:
							//Sol
						break;
						case 2:
							//AtP
						break;
						case 3:
							//x
						x = atof(word.c_str());
						break;
						case 4:
							//y
						y = atof(word.c_str());
						break;
						case 5:
							//z
						z = atof(word.c_str());
						break;
						case 6:
							//energy
						en = atof(word.c_str());
						break;
						case 7:
						r90 = atof(word.c_str());
						break;
						case 8:
							//type & save
						addData(x, y, z, en, r90, word);
						col = -1;
						break;
					}	
					col++;
					
					
				}

			}
			myfile.close();
			printf("%s\n","Fin de lectura de archivo de WS" );
		}else{
			printf("%s\n","Error de lectura de archivo de WS" );
		}
		//En el objeto voy a guardar el centro en coordenadas de grilla, para esto convierto las unidades reales a unidades de grilla.

	}

	void addMetadata(const char* filename,  double cx, double cy, double cz){
		std::ifstream myfile(filename);
		if (myfile.is_open())
		{
			std::string word;
			while (myfile >> word )
			{
				cantWs++;
			}
		}else{
			printf("%s\n","Error de lectura de archivo de WS" );
		}
		//Resto el header
		cantWs = cantWs / 9 - 1;
		listWs = new cubo[cantWs];
		centro_grilla_x = double(int(cx / resolucion));
		centro_grilla_y = double(int(cy / resolucion));
		centro_grilla_z = double(int(cz / resolucion)); 
		printf("%s centro grilla = (%f, %f, %f) | #ws = %d  \n","[WS] GridTransformer inicializado: ",centro_grilla_x, centro_grilla_y, centro_grilla_z, cantWs);

	}


		//Esta funcion recibe la linea del *.dat de WS que representa un ws: genera un cubo representante y guarda todos los datos de la linea
	void addData(double x, double y, double z, double en, double r90, std::string mp){

		 //Obtengo las coordenadas del agua y sus energias asi como el tipo de atomo que modifica
		//transformo esas unidades a unidades de grilla (osea aproximo el centro del ws a el punto mas cercano)
		double nx = double(int(x/resolucion + 0.5f));
		double ny = double(int(y/resolucion + 0.5f));
		double nz = double(int(z/resolucion + 0.5f));
	 	//TODO: ¿Como calculo este radio, es asi como se hace?
		double radio = double(int(r90/resolucion + 0.5f));

		//Calculo los puntos limites del cubo que representa al ws en coordenadas de la grilla
		double x_min = nx - radio - centro_grilla_x;
		double y_min = ny - radio - centro_grilla_y;
		double z_min = nz - radio - centro_grilla_z;
		double x_max = nx + radio - centro_grilla_x;
		double y_max = ny + radio - centro_grilla_y;
		double z_max = nz + radio - centro_grilla_z;
	//Asigno las variables calculadas al objeto guardado en la posicion last_cubo
	//FIXME: Por alguna razon estalla al descomentar x_max e y_min

		listWs[last_cubo].x_min = x_min;
	//listWs[last_cubo].x_max = x_max;
		listWs[last_cubo].xmax = x_max;
	//listWs[last_cubo].y_min = y_min;
		listWs[last_cubo].y_max = y_max;
		listWs[last_cubo].ymin = y_min;
		listWs[last_cubo].z_min = z_min;
		listWs[last_cubo].z_max = z_max;
		listWs[last_cubo].zc = z;
		listWs[last_cubo].en = en;
		listWs[last_cubo].type = mp;
	
		printf("[SETUP WS] Limites de cubo WS = %f <= x <= %f | %f <= y <= %f | %f <= z <= %f \n",listWs[last_cubo].x_min, listWs[last_cubo].xmax, listWs[last_cubo].ymin,  listWs[last_cubo].y_max, listWs[last_cubo].z_min,listWs[last_cubo].z_max);


		last_cubo++; 
	}

		//Determina si las coordenadas de grilla que se le pasan como parametro estan dentro de algun WS
		//Devuelve el cubo que representa al WS si es que el punto se encuentra dentro, caso contrario NULL
	cubo* in_ws(double x, double y, double z, std::string type){
				// printf("%s\n","Evaluando pertenencia" );
		int i = 0;
		for (i = 0; i < cantWs; i++)
		{
		//[TO TEST]
			printf("[SETUP WS] Limites de cubo WS = %f <= %f <= %f | %f <= %f <= %f | %f <= %f <= %f \n",listWs[i].x_min,x, listWs[i].xmax, listWs[i].ymin,y,  listWs[i].y_max, listWs[i].z_min,z,listWs[i].z_max);
			if (listWs[i].isType(type) && x >= listWs[i].x_min && x <= listWs[i].xmax && y >= listWs[i].ymin && y <= listWs[i].y_max && z >= listWs[i].z_min && z <= listWs[i].z_max)
			{
				break; 
			}
		}
		if(i!=cantWs){
			printf("[IN WS](%f, %f, %f)\n", x, y, z);
			return &listWs[i];
		}
		return NULL;
	}

		//Dado un punto de la grilla, determina si esta dentro de un cubo de WS, de estarlo aplica la modificacion a la energia, de lo contrario deja la energia tal cual estaba
	double modify(double x, double y, double z, double e, char type[3]){

		double em = e;
		std::string str(type);
		Cubo* cubo = in_ws(x, y, z, str);
		if (cubo != NULL)
		{
				//Primero recalculo el centro del ws... podria guardaarmelo, pero surgio un problema con c++ que al agregar mas variables arroja segmentation fault :(
				//FIX: No necesito transfrmar a datos reales, ya que la distancia se va a conservar
				double xc = ((cubo->xmax + cubo->x_min) / 2 /* + centro_grilla_x*/) /** resolucion*/;
				double yc = ((cubo->y_max + cubo->ymin) / 2 /*+ centro_grilla_y*/)/* * resolucion*/;
				double zc = ((cubo->z_max + cubo->z_min) / 2/* + centro_grilla_z*/)/* * resolucion*/;

						//Luego calculo el punto real que representa el punto en la grilla que me pasan
						//TODO: Fijarse si aca no hay que corregir tambien el centro como lo hacemos arriba
			// double px = (x /*+ centro_grilla_x*/)/* * resolucion*/;
			//  double py = (y /*+ centro_grilla_y*/)/** resolucion*/;
			//  double pz = (z /*+ centro_grilla_z*/)/* * resolucion*/;

						//Con los datos en coordenadas reales vamos a hacer los calculos para la modificacion de la energia en el punto
			double dist = sqrt(pow(xc - x,2)+pow(yc - y,2)+pow(zc - z,2));
				//printf("[WS DIST]:  xc=%f x=%f , yc=%f y=%f, zc=%f z=%f, dist = %f \n",xc,x,yc,y,zc,z,dist);

//No funciona  la gausiana pq es gato y cobra
				em = e + cubo->en ;//* exp(-(dist/cubo->e6));

				printf("[WS MOD]:  e_i = %f, e_f = %f, dist a WS = %f , en = %f,e_f - e_i = %f \n", e, em, dist, cubo->en, em - e /*(cubo->en * exp(-(dist/cubo->e6)))*/ );
			}else{
						printf("out\n");

						//Agregar aca si sse quiere modificar la energia de los puntos que no esten cerca de un ws
			}

			return em;
		}

	}GridTransformer;

	int main( int argc, const char* argv[] )
	{

		const char* WSF = get_w_parameter(argc, argv);
		GridTransformer gt_carlos (0, 0, 0, 0.1, WSF);

		gt_carlos.modify(700, 222, 270, 20, "OA");
	}
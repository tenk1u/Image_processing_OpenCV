/*
	Parte 1 del examen parcial
	Este programa divide una imagen en 16 bloques (4x4)
	aplica un filtro a cada bloque y luego vuelve a unirlos
	La aplicacion de filtros se hace en paralelo
	Finalmente, calcula el tiempo de ejecucion del filtrado
	y el tiempo de ejecucion total del programa
*/

#include<iostream>
#include<vector>
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<omp.h>

// Divide la imagen en COLxROW bloques
std::vector<cv::Mat> dividirImagen(cv::Mat image, int COLS, int ROWS){

	// Obtener el tamaño de la imagen
	int img_height, img_width;
	img_height = image.rows;
	img_width = image.cols;
	std::cout << cv::format("Tamaño de la imagen: %ix%i pixeles",img_height,img_width)<<std::endl;
	
	// Obtener el tamaño de los bloques
	int block_height, block_width;
	block_height = img_height/ROWS;
	block_width = img_width/COLS;
	std::vector<cv::Mat> bloques;

	// Generar bloques y agregarlos al vector 
	
	for(int y=0;y<ROWS;y++)
	{
		for(int x=0;x<COLS;x++)
		{
			cv::Rect roi( block_width  * x,
					block_height * y,
					block_width,
					block_height);
				bloques.push_back( image( roi ) );
		}
	}
	return bloques;
}

cv::Mat unirBloques(std::vector<cv::Mat> bloques, int COLS, int ROWS)
{
	cv::Mat imagen, tmp2,tmp4,tmp8;
	std::vector<cv::Mat> sub_8blq;
	std::vector<cv::Mat> sub_4blq;
	std::vector<cv::Mat> sub_2blq;

	for (int i = 0; i < COLS*ROWS ; i+=2)
	{
		tmp2 = cv::Mat();
		cv::hconcat(bloques[i], bloques[i+1],tmp2);
		sub_2blq.push_back(tmp2);
	}

	for (int i = 0; i < COLS*ROWS/2; i+=2)
	{
		tmp4 = cv::Mat();
		cv::hconcat(sub_2blq[i], sub_2blq[i+1],tmp4);
		sub_4blq.push_back(tmp4);
	}

	for (int i = 0; i < COLS*ROWS/4; i+=2)
	{
		tmp8 = cv::Mat();
		cv::vconcat(sub_4blq[i], sub_4blq[i+1],tmp8);
		sub_8blq.push_back(tmp8);
	}
	
	cv::vconcat(sub_8blq[0],sub_8blq[1],imagen);
	
	return imagen;
}	

cv::Mat processPixels(cv::Mat img, cv::Vec3f filter)
{
	cv::Mat prod = img;
	cv::Vec3b color;

	#pragma omp parallel for collapse(2)
	for(int rows=0;rows<prod.rows;rows++)
	{
		for(int cols=0; cols<prod.cols;cols++)
		{
			color=prod.at<cv::Vec3b>(rows,cols);
			
			color[0]=color[0]*filter[0];
			color[1]=color[1]*filter[1];
			color[2]=color[2]*filter[2];

			prod.at<cv::Vec3b>(rows,cols)=color;
		}
	}

	return prod;

}

int main(int argc, char** argv){

	// Inicializacion de variables
	cv::String img_file = "./imagenes/test2.jpg";
	double inicio_ejecucion, tiempo_ejecucion;
	double inicio_filtro_hilos, fin_filtro_hilos;
	double inicio_filtro_tareas, fin_filtro_tareas;
	const int COL_BLOQUES = 4;
	const int ROW_BLOQUES = 4; 

	// Inicio del tiempo de ejecucion
	inicio_ejecucion = omp_get_wtime();

	/* Tarea 1:  Cargar la imagen */
	cv::Mat image = cv::imread(img_file);

    // Mensaje de error si no se encuentra la imagen
    if (image.empty()) 
    {
        std::cout << "No se encontro la imagen" << std::endl;
    }

	/* Tarea 2: Dividir la imagen */
	std::vector<cv::Mat> bloques = dividirImagen(image, COL_BLOQUES, ROW_BLOQUES);

	/* Tarea 3: Filtrado */
	// Creando kernels para filtros
	std::vector<cv::Mat>kernels;
	std::vector<cv::Vec3f>filtros;

	float kernels_filtros[16][9] = {
		{0,0,0,0,1,0,0,0,0},	// Filtro 1
		{-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Filtro 2
		{0,-1,0,-1,5,-1,0,-1,0}, // Filtro 3
		{-1,-1,-1,-1,9,-1,-1,-1,-1}, // Filtro 4
		{0,-0.025,0,0,0.25,0,0,0.025,0}, // Filtro 5
		{0.-025,0,0,-0.025,0,0,-0.25,0,0}, // Filtro 6
		{0,0,0.04,0,0,0.7,0,0,0.04}, // Filtro 7
		{0.04,0.04,0.04,0.04,0.04,0.04,0.04,0.04,0.04}, // Filtro 8
		{0.1,0.5,0.1,0.5,0.15,0.5,0.1,0.5,0.1}, // Filtro 9
		{-1, 0, 1, -2, 0, 2, -1, 0, 1}, // Filtro 10
		{1020,0,-1020,1020,0,-1020,1020,0,-1020	}, // Filtro 11
		{0,0,0,0,1,0,0,0,0}, // Filtro 12
		{0,-1,0,-1,5,-1,0,-1,0}, // Filtro 13
		{0.1,0.5,0.1,0.5,0.15,0.5,0.1,0.5,0.1}, // Filtro 14
		{1020,1020,1020,0,0,0,-1020,-1020,-1020}, // Filtro 15
		{255,0,-255,255,0,-255,255,0,-255}, // Filtro 16
	};

	float valores_filtros[16][3] = {
		{1,1,1},
		{1,0,0},
		{0,1,0},
		{0,0,1},
		{1,1,0},
		{1,0,1},
		{0,1,1},
		{0.8,0.5,1},
		{0.2,1,1.4},
		{0,0.2,1.7}, 
		{0.5,0.5,0.5}, 
		{0.1,2,1}, 
		{0,0,0},	
		{2,0.2,0.1}, 
		{0,0.1,0.9}, 
		{0.5,0.5,0.5}, 
	};
	
	
	for(int i=0; i<COL_BLOQUES*ROW_BLOQUES; i++)
	{
		kernels.push_back(cv::Mat(3,3,CV_32F,kernels_filtros[i]));
	}

	for(int i=0; i<COL_BLOQUES*ROW_BLOQUES; i++)
	{
		filtros.push_back(cv::Vec3f(valores_filtros[i][0],valores_filtros[i][1],valores_filtros[i][2]));
	}

	/* Paralelismo de hilos */
	std::vector<cv::Mat>bloques_filtrados(16);

	// Inicia tiempo de filtro en hilos
	inicio_filtro_hilos = omp_get_wtime();

	#pragma omp parallel for
	for(int i = 0; i<bloques.size();i++)
	{
	    //cv::filter2D(bloques[i], bloques_filtrados[i], -1 , kernels[i]);
		bloques_filtrados[i] = processPixels(bloques[i],filtros[i]);
	}

	// Calculo del tiempo de filtro en hilos
	inicio_filtro_hilos = omp_get_wtime() - inicio_filtro_hilos;
	std::cout << "Tiempo aplicando filtros (hilos): " << inicio_filtro_hilos << std::endl;

	cv::Mat imagen_multifiltro = unirBloques(bloques_filtrados, COL_BLOQUES, ROW_BLOQUES);
	cv::imwrite("./imagenes/producto_hilos.jpg", imagen_multifiltro);

	// Calculo del tiempo total de ejecucion
	tiempo_ejecucion = omp_get_wtime() - inicio_ejecucion;
	std::cout << "Tiempo de ejecucion: " << tiempo_ejecucion << std::endl;
	
}

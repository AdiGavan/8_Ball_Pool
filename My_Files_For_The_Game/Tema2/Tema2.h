#pragma once
// "Copyright [2018] Gavan Adrian-George, 334CA"
#include <Component/SimpleScene.h>
#include "LabCamera.h"

// Structura pentru informatiile unei bile
struct info_ball {
	// Culoarea bilei
	std::string type;
	// Tipul de shader
	std::string shader_type;
	// Vectorul miscarii
	glm::dvec3 movement = glm::vec3(0, 0, 0);
	// Vectorul miscarii inainte sa fie schimbat in detectarea coliziunii
	glm::dvec3 movementInit = glm::vec3(0, 0, 0);
	// Coordonatele centrului bilei
	float xc;
	float yc;
	float zc;
};

class Tema2 : public SimpleScene
{
	public:
		Tema2();
		~Tema2();

		void Init() override;

	private:
		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;
		void RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix) override;
		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;
		void New_movement(int ball1, int ball2);
		bool Detect_collision(int ball1, int ball2);
		void Add_Meshes_Shaders();
		void Show_Statistics();
		void Add_Balls();
		void Add_Holes();
		void Render_Table();

	protected:
		// Camera
		Tema2Camera::Camera *camera;
		glm::mat4 projectionMatrix;
		// Vectorul ce contine bilele
		std::vector<info_ball> balls;
		// Vectorul ce contine bilele in pozitia initiala pentru un alt joc
		std::vector<info_ball> ballsInitial;
		// Vectorul ce contine gaurile mesei
		std::vector<info_ball> holes;
		// Dimensiunile mesei
		float const TableWidth = 3.0f;
		float const TableThickness = 0.4f;
		float const TableLength = 4.5f;
		float const TableHeight = 0.8f;

		// Flag pentru a sa stii cand se pregateste pentru lovitura
		bool preparing = false;
		// Flag pentru a sa cunoaste daca este pria lovitura din jocul curent
		bool first_hit = true;
		// Flag pentru a sti daca pozitia camerei este deja "top_view", pentru a nu o modifica mereu cand trebuie sa fie top_view
		bool top_view = false;

		// Variabile utilizate pentru coliziunile bilelor
		double deltaXSquared;
		double deltaZSquared;

		// Flag pentru a sti daca acum jucatorul regleaza 
		// directia loviturii (sa nu se execute miscarea tacului fata-spate pana nu s-a stabilit directia de lovire)
		bool aim = true;
		// Flag pentru a sti daca click stanga este apasat
		bool button_left = false;

		// Contor pentru a sti cand toate bilele de pe masa s-au oprit din miscare
		int contor_oprire_bile = 0;
		// Variabila utilizata in cazul intrarii bilei albe in gaura
		info_ball white_ball;
		// Flag pentru a sti daca s-a produs un fault (daca bila alba a intrat in gaura)
		bool fault = false;
		// Flag pentru a sti daca bila alba a lovit o bila sau daca a lovit bila care trebuie
		bool white_collision = false;
		
		// Jucatorul curent
		int player = 1;
		// Jucatorul care a inceput jocul curent
		int player_initial = 1;
		// Tipul ultimei mingi intrate in gaura
		std::string last_ball;
		// Flag pentru a sti daca un joc s-a terminat si trebuie inceput altul
		bool restart = false;

		// Flag pentru a sti daca jucatorii si-au ales culorile
		bool chosen_color = false;
		// Culoarea primului jucator
		std::string color_player1 = "X";
		// Culoarea celui de-al doilea jucator
		std::string color_player2 = "X";
		// Contor pentru bilele galbene
		int counter_bG = 0;
		// Contor pentru bilele rosii
		int counter_bR = 0;
		// Contor pentru a stii cate bile galbene au fost initial pentru refacerea jocului
		int counter_bG_init = 0;
		// Contor pentru a stii cate bile rosii au fost initial pentru refacerea jocului
		int counter_bR_init = 0;
		// Contor pentru numarul de faulturi al primului jucator
		int counter_faults_pl1 = 0;
		// Contor pentru numarul de faulturi al jucatorului 2
		int counter_faults_pl2 = 0;
		// Contor pentru numarul de victorii al primului jucator
		int victories_player1 = 0;
		// Contor pentru numarul de victorii al jucatorului 2
		int victories_player2 = 0;
		// Flag pentru a vedea daca in tura curenta jucatorul a bagat o bila de aceeasi culoare
		bool same_color_in = false;
		// Flag pentru a sti daca este prima oara in tura curenta cand bila alba loveste o bila colorata
		bool first_collision = false;
		// Flag pentru a sti daca este fault si vederea trebuie sa ramana top_down
		bool arange_ball = true;

		// deltaTimeSeconds anterior
		float deltaTimeSecondsPrevious;
		
		// Timpul trecut. Ajuta la miscarea tacului fata-spate si calcularea intensitatii loviturii
		float timeElapsed;

		// Unghiul de rotatie al tacului in jurul bilei albe
		float rotation_angle = 0;

		// Unghiul de vedere al camerei
		float fov;

		// Proprietati lumina si materiale
		glm::vec3 lightPosition;
		unsigned int materialShininess;
		float materialKd;
		float materialKs;
		
};

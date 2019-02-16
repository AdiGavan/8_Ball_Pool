// "Copyright [2018] Gavan Adrian-George, 334CA"
#include "Tema2.h"

#include <vector>
#include <string>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

void Tema2::Init()
{
	fov = 60;
	camera = new Tema2Camera::Camera();
	camera->Set(glm::vec3(0, 5, 0.000001f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
	projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, 0.01f, 200.0f);

	lightPosition = glm::vec3(0, 3, 0);
	materialShininess = 30;
	materialKd = 2;
	materialKs = 1;

	Add_Meshes_Shaders();
	Add_Balls();
	Add_Holes();
	Show_Statistics();

}

void Tema2::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::Update(float deltaTimeSeconds)
{

	// Daca se pregateste de lovitura, pune camera pe bila alba
	if (preparing) {
		camera->Set(glm::vec3(balls[balls.size() - 1].xc, balls[balls.size() - 1].yc, balls[balls.size() - 1].zc) - camera->GetForDist(), glm::vec3(balls[balls.size() - 1].xc, balls[balls.size() - 1].yc, balls[balls.size() - 1].zc), glm::vec3(0, 1, 0));
	}
	// Cand bila alba trebuie pozitionata sau se vizualizeaza lovitura
	else if (!top_view) {
		camera->Set(glm::vec3(0, 5, 0.000001f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		top_view = true;
	}

	Render_Table();

	// Se calculeaza viteza bilelor aflate in miscare
	if (!preparing && !arange_ball) {
		for (int i = 0; i < balls.size(); i++) {
			// Se scade cu 0.005 * movement pentru ca masa are frecare
			balls[i].movement.x = balls[i].movement.x - 0.005 * balls[i].movement.x;
			balls[i].movement.z = balls[i].movement.z - 0.005 * balls[i].movement.z;
			// Cand bila se misca foarte foarte incet, se opreste
			if ((fabs(balls[i].movement.x) <= 0.0003) && (fabs(balls[i].movement.z) <= 0.0003)) {
				balls[i].movement = glm::dvec3(0, 0, 0);
			}
			// Se imparte la deltaTime anterior pentru a avea valoarea puterii si se inmulteste
			// cu deltaTimeSeconds curent pentru ca miscarea sa fie in functie de frame-urile curente
			balls[i].movement = balls[i].movement / (double)deltaTimeSecondsPrevious * (double)deltaTimeSeconds;
		}
	}

	// Se retine deltaTime-ul curent
	deltaTimeSecondsPrevious = deltaTimeSeconds;

	// Dupa executarea loviturii (bilele sunt in miscare)
	if (!preparing && !arange_ball) {
		// In caz de ciocnire cu manta, se modifica vectorul de miscare
		for (int i = 0; i < balls.size(); i++) {
			// Loveste manta superioara
			if ((balls[i].zc + balls[i].movement.z) <= (-(TableLength / 2) + 0.22)) {
				balls[i].movement.z = -1 * balls[i].movement.z;
			}
			// Loveste manta inferioara
			if ((balls[i].zc + balls[i].movement.z) >= (TableLength / 2) - 0.22) {
				balls[i].movement.z = -1 * balls[i].movement.z;
			}
			// Loveste manta din stanga
			if ((balls[i].xc + balls[i].movement.x) <= (-(TableWidth / 2) + 0.22)) {
				balls[i].movement.x = -1 * balls[i].movement.x;
			}
			// Loveste manta din dreapta
			if ((balls[i].xc + balls[i].movement.x) >= (TableWidth / 2) - 0.22) {
				balls[i].movement.x = -1 * balls[i].movement.x;
			}
		}

		// Se verifica daca apar ciocniri (fiecare bila cu fiecare bila)
		for (int i = 0; i < balls.size() - 1; i++) {
			for (int j = i + 1; j < balls.size(); j++) {
				// Transmit la "Detect_collision" bila mai rapida ca prim parametru
				// Daca este coliziune, se calculeaza noii vectori de miscare
				if (length(balls[i].movement) > length(balls[j].movement)) {
					if (Detect_collision(i, j)) {
						New_movement(i, j);
					}
				}
				else {
					if (Detect_collision(j, i)) {
						New_movement(j, i);
					}
				}
			}
		}

		// Se verifica daca o bila intra in gaura
		for (int i = 0; i < balls.size(); i++) {
			for (int j = 0; j < holes.size(); j++) {
				deltaXSquared = balls[i].xc - holes[j].xc;
				deltaXSquared *= deltaXSquared;
				deltaZSquared = balls[i].zc - holes[j].zc;
				deltaZSquared *= deltaZSquared;
				// O bila colorata are raza de 0.06 si o gaura are raza de 0.10
				// S-a pus 0.14 ca bila colorata sa intre putin spre gaura inainte sa se considere intrata in gaura
				// Aceasta conditie verifica daca o bila a intrat in gaura
				if (deltaXSquared + deltaZSquared <= 0.14 * 0.14) {
					// Daca o bila colorata a intrat in gaura si jucatorii nu si-au ales culorile,
					// se seteaza culoarea fiecaruia
					if ((balls[i].type != "A") && !chosen_color) {
						// Sa nu fie nici bila neagra cea care a intrat
						if (balls[i].type != "N") {
							if (player == 1) {
								color_player1 = balls[i].type;
								if (balls[i].type == "R") {
									color_player2 = "G";
								}
								else {
									color_player2 = "R";
								}
							}
							else {
								color_player2 = balls[i].type;
								if (balls[i].type == "R") {
									color_player1 = "G";
								}
								else {
									color_player1 = "R";
								}
							}
							// Se retine faptul ca jucatorii au acum culorile alese
							chosen_color = true;
						}
					}

					// Se verifica daca un jucator a bagat o bila care ii apartine
					if (player == 1) {
						if (balls[i].type == color_player1) {
							same_color_in = true;
						}
					}
					else if (player == -1) {
						if (balls[i].type == color_player2) {
							same_color_in = true;
						}
					}

					// Se scade numarul total de bile ramase din culoarea bilei intrate
					if (balls[i].type == "R") {
						counter_bR--;
					}
					else if (balls[i].type == "G"){
						counter_bG--;
					}
					
					// Se verifica daca bila alba a intrat in gaura
					// Se pune &&(!fault) ca sa nu cumva sa se retina si bile colorate,
					// pentru ca bila alba va fi scoasa => noul balls.size() - 1 va fi o bila colorata
					if ((i == balls.size() - 1) && (!fault)) {
						// Se retine bila alba si se retine faptul ca a fost facut un fault
						white_ball = balls[i];
						fault = true;
					}
					// Se retine tipul ultimei bile intrate in gaura si se sterge bila din vectorul bilelor
					last_ball = balls[i].type;
					balls.erase(balls.begin() + i);

					// Daca bila neagra a fost bagata in gaura
					if (last_ball == "N") {
						// Se verifica daca jucatorul a castigat sau a pierdut
						// Cazul cand primul jucator introduce bila neagra
						if (player == 1) {
							// Daca a introdus bila neagra cand trebuia si nu a facut fault in timpul loviturii
							// => el a castigat
							if (color_player1 == "N" && !fault && white_collision) {
								restart = true;
								victories_player1++;
							}
							// Altfel celalalt jucator a castigat
							else {
								restart = true;
								victories_player2++;
							}
						}
						// Cazul cand al 2-lea jucator introduce bila neagra
						else if (player == -1) {
							// Daca a introdus bila neagra cand trebuia si nu a facut fault in timpul loviturii
							// => el a castigat
							if (color_player2 == "N" && !fault && white_collision) {
								restart = true;
								victories_player2++;
							}
							// Altfel celalalt jucator a castigat
							else {
								restart = true;
								victories_player1++;
							}
						}
					}
					break;
				}
			}
		}
	
	}

	contor_oprire_bile = 0;
	// Daca s-a executat lovitura (bilele sunt in miscare)
	if (!preparing && !arange_ball) {
		for (int i = 0; i < balls.size(); i++) {
			// Daca bila se misca, se calculeaza noua pozitie a bilei
			if (balls[i].movement != glm::dvec3(0,0,0)) {
			
				balls[i].xc += balls[i].movement.x;
				balls[i].zc += balls[i].movement.z;

			}
			// Daca bila sta pe loc, se incrementeaza contorul de oprire
			else {
				contor_oprire_bile++;
			}	
		}
	}
	
	// Daca inca se urmareste efectul loviturii si toate toate bilele s-au oprit din miscare (lovitura s-a terminat)
	if (!preparing && !arange_ball && contor_oprire_bile == balls.size()) {
		aim = true;
		button_left = false;

		// Daca a fost fault => jucatorul urmator poate pune bila oriunde pe masa
		if (fault || (!white_collision)) {
			arange_ball = true;
		}
		// Atlfel, se modifica camera si se retine faptul ca urmatorul jucator isi va pregati lovitura
		else {
			preparing = true;
			camera->Set(glm::vec3(0, 3, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		}

		// Se reseteaza unghiul de rotire al tacului
		rotation_angle = 0;
		
		// Se verifica daca un jucator a introdus toate bilele lui si trebuie sa loveasca bila neagra
		// Se fac aici verificarile pentru ca daca se faceau anterior nu se evita cazul
		// ultima bila corecta => se schimba culoarea pe negru => intra si bila neagra imediat (in aceeasi tura 
		// => in loc sa piarda jucatorul castiga
		// Daca bilele unui jucator s-au terminat, contorul bilelor corespunzatoare este setat -1
		// pentru a nu se intra din nou pe acelasi caz
		if (counter_bR == 0) {
			counter_bR = -1;
			if (color_player1 == "R") {
				color_player1 = "N";
			}
			else {
				color_player2 = "N";
			}
		}
		else if (counter_bG == 0) {
			counter_bG = -1;
			if (color_player1 == "G") {
				color_player1 = "N";
			}
			else {
				color_player2 = "N";
			}
		}
		// Daca s-a comis un fault, se incrementeaza numarul de greseli al jucatorului corespunzator
		if (fault || (!white_collision)) {
			if (player == 1) {
				counter_faults_pl1++;
			}
			else {
				counter_faults_pl2++;
			}
		}
		// Daca jucatorul curent a introdus si o bila de a lui in gaura
		if (same_color_in) {
			// Daca jucatorul curent nu a produs fault si bila alba a lovit prima oara o bila corecta
			// acelasi jucator loveste din nou
			if (!fault && white_collision) {
				player = player;
			}
			else {
				player *= -1;
			}
		}
		// Daca jucatorul nu a introdus o bila de-a lui in gaura se schimba jucatorul
		else {
			player *= -1;
		}

		// Daca jocul curent s-a terminat si trebuie inceput un joc nou
		if (restart) {
			preparing = false;
			camera->Set(glm::vec3(0, 5, 0.000001f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
			balls = ballsInitial;
			counter_bG = counter_bG_init;
			counter_bR = counter_bR_init;
			fault = false;
			white_collision = false;
			same_color_in = false;
			first_hit = true;
			restart = false;
			// Fiecare joc nou este inceput de alt jucator (o data jucatorul 1, urmatorul joc incepe jucatorul 2)
			player = -1 * player_initial;
			player_initial = player;
			chosen_color = false;
			color_player1 = "X";
			color_player2 = "X";
			counter_faults_pl1 = 0;
			counter_faults_pl2 = 0;
			arange_ball = true;
			
			// Daca a fost facut fault (fault aici contorizeaza doar daca bila alba a intrat in gaura) 
		} else if (fault) {
			// Se pune bila inapoi pe masa
			balls.push_back(white_ball);
			balls[balls.size() - 1].xc = 0;
			balls[balls.size() - 1].zc = (TableLength / 2) - ((TableLength / 3) / 2);
		}

		Show_Statistics();

	}

	// Afisare bile
	for (int i = 0; i < balls.size(); i++) {
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(balls[i].xc, balls[i].yc, balls[i].zc));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.12f));
		RenderMesh(meshes["sphere"], shaders[balls[i].shader_type], modelMatrix);
	}

	// Afisare tac
	if (preparing) {
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(balls[balls.size()-1].xc, balls[balls.size() - 1].yc + 0.01f, balls[balls.size() - 1].zc));
		// Daca nu regleaza directia, se trimite timeElapsed (pentru animatia tacului) in caz ca jucatorul vrea sa loveasca
		if (!aim) {
			timeElapsed = Engine::GetElapsedTime();
		}
		// Altfel se trimite -1;
		else {
			timeElapsed = -1;
		}

		modelMatrix = modelMatrix = glm::rotate(modelMatrix, RADIANS(rotation_angle), glm::vec3(0, 1, 0));
		modelMatrix = modelMatrix = glm::rotate(modelMatrix, RADIANS(-17), glm::vec3(1, 0, 0));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.6f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.030f, 0.021f, 1.0f));

		RenderMesh(meshes["box"], shaders["ShaderTac"], modelMatrix);

		// Afisare linii ghidare
		for (int i = 0; i < 8; i++) {
			modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(balls[balls.size() - 1].xc, balls[balls.size() - 1].yc, balls[balls.size() - 1].zc));
			modelMatrix = modelMatrix = glm::rotate(modelMatrix, RADIANS(rotation_angle), glm::vec3(0, 1, 0));
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.2f + (-0.4f) * i));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.010f, 0.021f, 0.2f));

			RenderMesh(meshes["box"], shaders["ShaderLiniiGhidare"], modelMatrix);
		}

	}

	// Afisare gauri
	for (int i = 0; i < holes.size(); i++) {
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(holes[i].xc, holes[i].yc, holes[i].zc));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
		RenderMesh(meshes["sphere"], shaders[holes[i].shader_type], modelMatrix);
	}
}

void Tema2::FrameEnd()
{
	DrawCoordinatSystem(camera->GetViewMatrix(), projectionMatrix);
}

void Tema2::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
	if (!mesh || !shader || !shader->program)
		return;

	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Se trimit valorile pentru calculul luminii
	glm::vec3 eyePosition = camera->GetPosition();
	int eye_position = glGetUniformLocation(shader->program, "eye_position");
	glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

	int light_position = glGetUniformLocation(shader->program, "light_position");
	glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

	int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(material_shininess, materialShininess);

	int material_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(material_kd, materialKd);

	int material_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(material_ks, materialKs);

	// Se trimite timeElapsed la shader.
	int location = glGetUniformLocation(shader->GetProgramID(), "timeElapsed");
	glUniform1f(location, timeElapsed);

	// Se trimite unghiul tacului fata de bila alba la shader.
	location = glGetUniformLocation(shader->GetProgramID(), "angle");
	glUniform1f(location, RADIANS(rotation_angle));

	mesh->Render();
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	// Daca este prima lovitura din joc si bila trebuie aranjata, 
	// se muta bila oriunde in prima treime a mesei.
	if (first_hit && arange_ball) {
		// Se muta bila in fata maxim pana cand ajunge la treimea mesei.
		if (window->KeyHold(GLFW_KEY_W)) {
			if ((balls[balls.size() - 1].zc - 1 * deltaTime) >= ((TableLength / 2) - (TableLength / 3))) {
				balls[balls.size() - 1].zc -= 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].zc = ((TableLength / 2) - (TableLength / 3));
			}
		}

		// Se muta bila in stanga maxim pana cand ajunge la manta din stanga a mesei.
		if (window->KeyHold(GLFW_KEY_A)) {
			if ((balls[balls.size() - 1].xc - 1 * deltaTime) >= -(TableWidth / 2) + 0.225) {
				balls[balls.size() - 1].xc -= 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].xc = -(TableWidth / 2) + 0.225;
			}

		}
		// Se muta bila in spate maxim pana cand ajunge la manta inferioara a mesei.
		if (window->KeyHold(GLFW_KEY_S)) {
			if ((balls[balls.size() - 1].zc + 1 * deltaTime) <= ((TableLength / 2) - 0.225)) {
				balls[balls.size() - 1].zc += 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].zc = ((TableLength / 2) - 0.225);
			}
		}

		// Se muta bila in stanga maxim pana cand ajunge la manta din dreapta a mesei.
		if (window->KeyHold(GLFW_KEY_D)) {
			if ((balls[balls.size() - 1].xc + 1 * deltaTime) <= (TableWidth / 2) - 0.225) {
				balls[balls.size() - 1].xc += 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].xc = (TableWidth / 2) - 0.225;
			}
		}
	}
	// Cazul cand a fost facut un fault si bila alba poate fi pusa oriunde pe masa
	else if (arange_ball) {
		// Se muta bila in fata maxim pana cand ajunge la manta superioara a mesei.
		if (window->KeyHold(GLFW_KEY_W)) {
			if ((balls[balls.size() - 1].zc - 1 * deltaTime) >= -(TableLength / 2) + 0.225) {
				balls[balls.size() - 1].zc -= 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].zc = -(TableLength / 2) + 0.225;
			}
		}
		// Se muta bila in stanga maxim pana cand ajunge la manta din stanga a mesei.
		if (window->KeyHold(GLFW_KEY_A)) {
			if ((balls[balls.size() - 1].xc - 1 * deltaTime) >= -(TableWidth / 2) + 0.225) {
				balls[balls.size() - 1].xc -= 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].xc = -(TableWidth / 2) + 0.225;
			}

		}
		// Se muta bila in spate maxim pana cand ajunge la manta inferioara a mesei.
		if (window->KeyHold(GLFW_KEY_S)) {
			if ((balls[balls.size() - 1].zc + 1 * deltaTime) <= ((TableLength / 2) - 0.225)) {
				balls[balls.size() - 1].zc += 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].zc = ((TableLength / 2) - 0.225);
			}
		}
		// Se muta bila in stanga maxim pana cand ajunge la manta din dreapta a mesei.
		if (window->KeyHold(GLFW_KEY_D)) {
			if ((balls[balls.size() - 1].xc + 1 * deltaTime) <= (TableWidth / 2) - 0.225) {
				balls[balls.size() - 1].xc += 1 * deltaTime;
			}
			else {
				balls[balls.size() - 1].xc = (TableWidth / 2) - 0.225;
			}
		}

	}

}

void Tema2::OnKeyPress(int key, int mods)
{
	// Cand se apasa SPACE (pentru a pozitiona bila pe masa)
	if (key == GLFW_KEY_SPACE) {
		bool ok = true;
		// Daca bila trebuie pusa pe masa
		if (arange_ball) {
			// Se verifica daca bila alba este pusa peste o alta bila
			// Daca este pusa peste o alta bila, flag-ul devine "false"
			for (int i = 0; i < balls.size() - 1; i++) {
				deltaXSquared = balls[balls.size() - 1].xc - balls[i].xc;
				deltaXSquared *= deltaXSquared;
				deltaZSquared = balls[balls.size() - 1].zc - balls[i].zc;
				deltaZSquared *= deltaZSquared;
				if (deltaXSquared + deltaZSquared <= 0.12 * 0.12) {
					ok = false;
				}
			}
			// Daca bila alba nu este pusa peste o alta bila, pozitionez bila alba
			if (ok) {
				button_left = false;
				aim = true;
				preparing = true;
				arange_ball = false;
				camera->Set(glm::vec3(0, 3, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
			}
		}
	}
	
}

void Tema2::OnKeyRelease(int key, int mods)
{
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Daca click dreapta este apasat sau ambele click-uri sunt apasate => jucatorul tinteste
	// => camera se poate roti si unghiul de rotire al tacului se modifica si el.
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) || (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) && window->MouseHold(GLFW_MOUSE_BUTTON_LEFT))) {
		aim = true;
		button_left = false;
		float sensivityOX = 0.001f;
		float sensivityOY = 0.001f;

		if (preparing) {
			rotation_angle += -deltaX * 0.0573f;
			camera->RotateThirdPerson_OY(-deltaX * sensivityOY);
		}

	}
	// Altfel, daca click stanga este apasat => jucatorul vrea sa loveasca
	else if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {
		button_left = true;
		aim = false;
	}

}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Daca click dreapta este apasat sau ambele click-uri sunt apasate => jucatorul regleaza tina
	if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT) || (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) && IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))) {
		aim = true;
		button_left = false;
	}
	// Altfel, daca doar click stanga este apasat => jucatorul vrea sa loveasca
	else if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
		button_left = true;
		aim = false;
	}

}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Daca doar butonul stang al mouse-ului este apasat
	if (preparing && (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) && button_left)) {
		glm::vec3 direction;
		direction = camera->GetDir();

		// Se muta [-1, 1] de la cos la [0,2] si se imparte la "distanta" maxima (sa fie proportionala lovitura 
		// cu distanta dintre tac si bila) si se inmulteste cu puterea maxima.
		float putere = ((cos(timeElapsed) + 1.0f) / 2.0f) * 5.0f;

		// Vectorul de miscare al bilei albe se inmulteste cu deltaTime ca sa fie in functie de frame-uri
		balls[balls.size() - 1].movement = putere * direction * deltaTimeSecondsPrevious;
		balls[balls.size() - 1].movementInit = putere * direction;
		
		fault = false;
		white_collision = false;
		preparing = false;
		first_hit = false;
		same_color_in = false;
		first_collision = false;
		top_view = false;
		aim = true;
		button_left = false;
	}

}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}

/* Functie pentru calcularea noilor vectori de miscare a bilelor dupa ciocnire */
void Tema2::New_movement(int ball1, int ball2) {

	glm::vec3 center1 = glm::vec3(balls[ball1].xc, balls[ball1].yc, balls[ball1].zc);
	glm::vec3 center2 = glm::vec3(balls[ball2].xc, balls[ball2].yc, balls[ball2].zc);
	glm::dvec3 n = center1 - center2;
	n = normalize(n);
	
	float a1 = dot(balls[ball1].movementInit, n);
	float a2 = dot(balls[ball2].movement, n);
	double optimizedP = (2.0 * (a1 - a2)) / (100 + 100);
	
	glm::vec3 v1 = balls[ball1].movementInit - optimizedP * 100 * n;
	glm::vec3 v2 = balls[ball2].movement + optimizedP * 100 * n;

	// Se inmulteste cu 0.9 pentru a mai scadea putin viteza dupa ciocnire (un fel de pierdere de energie)
	balls[ball1].movement = v1 * deltaTimeSecondsPrevious * 0.9f;
	balls[ball2].movement = v2 * deltaTimeSecondsPrevious * 0.9f;

}

/* Functie pentru detectarea coliziunilor */
bool Tema2::Detect_collision(int ball1, int ball2) {

	glm::dvec3 aux(balls[ball1].movement.x, balls[ball1].movement.y, balls[ball1].movement.z);

	// Bila 1 se misca mai repede
	glm::vec3 center1 = glm::vec3(balls[ball1].xc, balls[ball1].yc, balls[ball1].zc);
	glm::vec3 center2 = glm::vec3(balls[ball2].xc, balls[ball2].yc, balls[ball2].zc);
	double dist = distance(center2, center1);
	double sumRadii = 0.12;
	
	dist -= sumRadii;
	if (length(balls[ball1].movement) < dist) {
		return false;
	}

	glm::dvec3 n = balls[ball1].movement;
	n = normalize(n);
	glm::dvec3 c = center2 - center1;

	double d = dot(n, c);
	if (d <= 0) {
		return false;
	}

	double len_c = length(c);
	double f = (len_c * len_c) - (d * d);
	double sumRadiiSquared = sumRadii * sumRadii;
	
	if (f >= sumRadiiSquared) {
		return false;
	}

	double t = sumRadiiSquared - f;

	if (t < 0) {
		return false;
	}

	double distance = d - sqrt(t);

	double mag = length(balls[ball1].movement);

	if (mag < distance) {
		return false;
	}

	// Daca ajunge aici => au facut coliziune
	// Salvez valoarea curenta a miscarii pentru ca algoritmul de detectare
	// a coliziunii modifica viteza astfel incat la urmatorul pas bila sa 
	// aiba viteza astfel incat sa atinga exact cealalta bila
	balls[ball1].movementInit = aux / (double)deltaTimeSecondsPrevious;
	balls[ball1].movement = normalize(balls[ball1].movement);
	balls[ball1].movement = distance * balls[ball1].movement;

	// Daca este prima oara cand bila loveste o bila in tura curenta
	if (!first_collision) {
		// Daca una din bile este bila alba
		if ((ball1 == balls.size() - 1) || (ball2 == balls.size() - 1)) {
			// Daca culoarea jucatorilor nu a fost aleasa => orice bila loveste nu a facut fault
			if (!chosen_color) {
				white_collision = true;
			}
			// Daca s-au ales culorile se verifica daca culoarea primei bile din tura curenta
			// lovita de bila alba corespunde cu culoarea jucatorului.
			// Daca nu corespunde => fault
			else {
				if (player == 1) {
					if ((balls[ball1].type == color_player1) || (balls[ball2].type == color_player1)) {
						white_collision = true;
					}
				}
				else if (player == -1) {
					if ((balls[ball1].type == color_player2) || (balls[ball2].type == color_player2)) {
						white_collision = true;
					}
				}
			}
			first_collision = true;
		}
	}

	return true;
}

/* Functie pentru adaugarea bilelor */
void Tema2::Add_Balls() {
	
	info_ball aux;

	// Primul rand de bile
	for (int i = 0; i < 5; i++) {
		if (i % 2 == 1) {
			aux.type = "R";
			aux.shader_type = "ShaderBR";
			counter_bR++;
		}
		else {
			aux.type = "G";
			aux.shader_type = "ShaderBG";
			counter_bG++;
		}
		aux.xc = -0.24 + i * 0.13;
		aux.yc = 1.06f;
		aux.zc = -(TableLength / 3);
		balls.push_back(aux);
	}

	// Al doilea rand de bile
	for (int i = 0; i < 4; i++) {
		if (i % 2 == 0) {
			aux.type = "G";
			aux.shader_type = "ShaderBG";
			counter_bG++;
		}
		else {
			aux.type = "R";
			aux.shader_type = "ShaderBR";
			counter_bR++;
		}
		aux.xc = -0.18 + i * 0.13;
		aux.yc = 1.06f;
		aux.zc = -(TableLength / 3) + 0.115;
		balls.push_back(aux);
	}

	// Al 3-lea rand de bile
	for (int i = 0; i < 3; i++) {
		if (i == 0) {
			aux.type = "R";
			aux.shader_type = "ShaderBR";
			counter_bR++;
		}
		else if (i == 1) {
			aux.type = "N";
			aux.shader_type = "ShaderBN";
		}
		else {
			aux.type = "G";
			aux.shader_type = "ShaderBG";
			counter_bG++;
		}
		aux.xc = -0.12 + i * 0.13;
		aux.yc = 1.06f;
		aux.zc = -(TableLength / 3) + 0.23;
		balls.push_back(aux);
	}

	// Al 4-lea rand de bile
	for (int i = 0; i < 2; i++) {
		if (i == 0) {
			aux.type = "G";
			aux.shader_type = "ShaderBG";
			counter_bG++;
		}
		else if (i == 1) {
			aux.type = "R";
			aux.shader_type = "ShaderBR";
			counter_bR++;
		}
		aux.xc = -0.06 + i * 0.13;
		aux.yc = 1.06f;
		aux.zc = -(TableLength / 3) + 0.345;
		balls.push_back(aux);
	}

	// Ultima bila din pachet
	aux.type = "R";
	aux.shader_type = "ShaderBR";
	counter_bR++;
	aux.xc = 0;
	aux.yc = 1.06f;
	aux.zc = -(TableLength / 3) + 0.460;
	balls.push_back(aux);

	// Se retine numarul initial din fiecare tip de bila
	counter_bR_init = counter_bR;
	counter_bG_init = counter_bG;

	// Bila alba
	aux.type = "A";
	aux.shader_type = "ShaderBA";
	aux.xc = 0;
	aux.yc = 1.06f;
	aux.zc = (TableLength / 2) - ((TableLength / 3) / 2);
	balls.push_back(aux);

	// Se retine numarul initial de bile
	ballsInitial = balls;
}

/* Functie pentru adaugarea gaurilor */
void Tema2::Add_Holes() {

	info_ball aux;

	for (int i = 0; i < 6; i++) {
		aux.type = "N";
		aux.shader_type = "ShaderBN";
		if (i < 3) {
			aux.zc = -(TableLength / 2) + i * (TableLength / 2);
			aux.yc = 0.98f;
			aux.xc = -(TableWidth / 2 - 0.14);
		}
		else {
			aux.zc = -(TableLength / 2) + (i - 3) * (TableLength / 2);
			aux.yc = 0.98f;
			aux.xc = (TableWidth / 2 - 0.14);
		}
		if (i == 0 || i == 3) {
			aux.zc += 0.14;
		}
		else if (i == 2 || i == 5) {
			aux.zc -= 0.14;
		}

		if (i == 0) {
			aux.zc += 0.03;
			aux.xc += 0.03;
		}

		if (i == 2) {
			aux.zc -= 0.03;
			aux.xc += 0.03;
		}

		if (i == 3) {
			aux.zc += 0.03;
			aux.xc -= 0.03;
		}

		if (i == 5) {
			aux.zc -= 0.03;
			aux.xc -= 0.03;
		}

		holes.push_back(aux);
	}
}

/* Functie pentru adaugarea shader-urilor si meshe-urilor */
void Tema2::Add_Meshes_Shaders() {
	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Shader *shader = new Shader("ShaderMasa");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderMasa.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderManta");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderManta.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderPicioare");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderPicioare.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderBA");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderBilaAlba.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderBN");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderBilaNeagra.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderBR");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderBilaRosie.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderBG");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderBilaGalbena.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderTac");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShaderTac.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderTac.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader *shader = new Shader("ShaderLiniiGhidare");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShaderLiniiGhidare.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

/* Functie pentru afisare statistici */
void Tema2::Show_Statistics() {
	cout << "Player" << "   " << "Player_Color" << "   " << "Remaining_balls" << "   " 
		 << "Faults" << "   " << "Victories" << endl;
	cout << "  1   " << "   " << "     " << color_player1 << "         " << "       ";
	if (color_player1 == "R") {
		cout << counter_bR;
	}
	else if (color_player1 == "G") {
		cout << counter_bG;
	}
	else {
		cout << "X";
	}
	cout << "             " << counter_faults_pl1 << "         " << victories_player1 << endl;

	cout << "  2   " << "   " << "     " << color_player2 << "         " << "       ";
	if (color_player2 == "R") {
		cout << counter_bR;
	}
	else if (color_player2 == "G") {
		cout << counter_bG;
	}
	else {
		cout << "X";
	}
	cout << "             " << counter_faults_pl2 << "         " << victories_player2 << endl;

	cout << "Player to hit: ";

	if (player == 1) {
		cout << player << endl;
	}
	else {
		cout << "2" << endl;
	}

	cout << endl;
	cout << endl;
	cout << endl;
}

/* Functie pentru afisarea mesei */
void Tema2::Render_Table() {
	// Masa
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, TableHeight, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(TableWidth, TableThickness, TableLength));
		RenderMesh(meshes["box"], shaders["ShaderMasa"], modelMatrix);
	}

	// Picioare
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-(TableWidth / 2 - 0.05), 0.3f, (TableLength / 2 - 0.05)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
		RenderMesh(meshes["box"], shaders["ShaderPicioare"], modelMatrix);
	}

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3((TableWidth / 2 - 0.05), 0.3f, (TableLength / 2 - 0.05)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
		RenderMesh(meshes["box"], shaders["ShaderPicioare"], modelMatrix);
	}

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-(TableWidth / 2 - 0.05), 0.3f, -(TableLength / 2 - 0.05)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
		RenderMesh(meshes["box"], shaders["ShaderPicioare"], modelMatrix);
	}

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3((TableWidth / 2 - 0.05), 0.3f, -(TableLength / 2 - 0.05)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
		RenderMesh(meshes["box"], shaders["ShaderPicioare"], modelMatrix);
	}

	// Mante
	// Stanga-dreapta
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-(TableWidth / 2 - 0.07), 1.04f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.14f, 0.08f, TableLength));
		RenderMesh(meshes["box"], shaders["ShaderManta"], modelMatrix);
	}

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3((TableWidth / 2 - 0.07), 1.04f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.14f, 0.08f, TableLength));
		RenderMesh(meshes["box"], shaders["ShaderManta"], modelMatrix);
	}

	// Fata-spate
	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1.04f, (TableLength / 2 - 0.07)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(TableWidth, 0.08f, 0.14f));
		RenderMesh(meshes["box"], shaders["ShaderManta"], modelMatrix);
	}

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1.04f, -(TableLength / 2 - 0.07)));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(TableWidth, 0.08f, 0.14f));
		RenderMesh(meshes["box"], shaders["ShaderManta"], modelMatrix);
	}
}
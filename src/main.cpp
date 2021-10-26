#include <SDL.h>
#include <stdio.h>
#include <stdlib.h> 
#include "rendering/renderer.h"
#include "rendering/scene.h"
#include "Physics/Vector3D.h"
#include "Physics/forcesRegister.h"
#include "Physics/particleGravityGenerator.h"
#include "Physics/particleDampingGenerator.h"
#include "Physics/particleAnchoredSpringGenerator.h"
#include "Physics/particleSpringGenerator.h"

#include "Physics/ParticleContactSolver.h"
#include "Physics/ParticleCable.h"
#include "Physics/ParticleRod.h"

bool runGame = true;
float particleMass = 1;
float randomZDirection = 0;// 2.5f;
float particleLinkLength = 2;

ForcesRegister forcesRegister;
ParticleContactSolver particleContactSolver;

//############################
//#                          #
//#  CREATION DE PARTICULES  #
//#                          #
//############################

Particle* CreateParticle(Scene* scene, Vector3D pos, Vector3D velocity = Vector3D(0, 0, 0), float massFactor = 1)
{
	Particle* p = new Particle(pos, particleMass * massFactor);
	scene->AddParticle(p);
	p->SetVelocity(velocity);

	forcesRegister.AddEntry(p, new ParticleGravityGenerator());
	forcesRegister.AddEntry(p, new ParticleDampingGenerator());

	return p;
}

void CreateSpring(Scene* scene)
{
	//cr�ation des particles
	Particle* p1 = new Particle(Vector3D(-2, 2, 0), particleMass);
	scene->AddParticle(p1);
	Particle* p2 = new Particle(Vector3D(2, 2, 0), particleMass * 2);
	scene->AddParticle(p2);

	//cr�ation des forces
	forcesRegister.AddEntry(p1, new ParticleDampingGenerator());
	forcesRegister.AddEntry(p1, new ParticleSpringGenerator(p2, particleLinkLength));
	forcesRegister.AddEntry(p2, new ParticleDampingGenerator());
	forcesRegister.AddEntry(p2, new ParticleSpringGenerator(p1, particleLinkLength));
}

//cr�e deux particules reli�es par une tige
void CreateRod(Scene* scene)
{
	//cr�ation des particules
	CreateParticle(scene, Vector3D(-particleLinkLength * 0.5f, 2, 0), Vector3D(rand() % 10 - 5, rand() % 10 - 5, 0));
	CreateParticle(scene, Vector3D(particleLinkLength * 0.5f, 2, 0), Vector3D(rand() % 10 - 5, rand() % 10 - 5, 0));
	
	//on r�cup�re des pointeurs vers ces particules...
	Particle* first = scene->gameObjects[scene->gameObjects.size() - 2];
	Particle* second = scene->gameObjects[scene->gameObjects.size() - 1];

	//...puis on cr�e le lien
	ParticleRod* rod = new ParticleRod(first, second, particleLinkLength);
	particleContactSolver.generator.AddParticleLink(rod);
}

//cr�e deux particules reli�es par un cable
void CreateCable(Scene* scene)
{
	//cr�ation des particules
	CreateParticle(scene, Vector3D(-particleLinkLength * 0.5f, 2, 0), Vector3D(rand() % 10 - 5, rand() % 10 - 5, 0));
	CreateParticle(scene, Vector3D(particleLinkLength * 0.5f, 2, 0), Vector3D(rand() % 10 - 5, rand() % 10 - 5, 0));

	//on r�cup�re des pointeurs vers ces particules...
	Particle* first = scene->gameObjects[scene->gameObjects.size() - 2];
	Particle* second = scene->gameObjects[scene->gameObjects.size() - 1];

	//...puis on cr�e le lien
	ParticleCable* cable = new ParticleCable(first, second, particleLinkLength);
	particleContactSolver.generator.AddParticleLink(cable);
}

//cr�er un cube compos� de 8 particules reli�es par des tiges
void CreateRodCube(Scene* scene)
{
	float halfLen = particleLinkLength * 0.5f;

	//cr�ation des particules
	std::vector<Particle*> particles;
	particles.push_back(CreateParticle(scene, Vector3D(-halfLen, halfLen + 2, -halfLen), Vector3D(20, 10, rand() % 10 - 5))); //c�t� avant haut-gauche
	particles.push_back(CreateParticle(scene, Vector3D(halfLen, halfLen + 2, -halfLen), Vector3D(0, 0, 0))); // c�t� avant haut-droit
	particles.push_back(CreateParticle(scene, Vector3D(-halfLen, -halfLen + 2, -halfLen), Vector3D(0, 0, 0))); //c�t� avant bas-gauche
	particles.push_back(CreateParticle(scene, Vector3D(halfLen, -halfLen + 2, -halfLen), Vector3D(0, 0, 0))); //c�t� avant bas-droit
	particles.push_back(CreateParticle(scene, Vector3D(-halfLen, halfLen + 2, halfLen), Vector3D(0, 0, 0))); //c�t� arri�re haut-gauche
	particles.push_back(CreateParticle(scene, Vector3D(halfLen, halfLen + 2, halfLen), Vector3D(0, 0, 0))); // c�t� arri�re haut-droit
	particles.push_back(CreateParticle(scene, Vector3D(-halfLen, -halfLen + 2, halfLen), Vector3D(0, 0, 0))); //c�t� arri�re bas-gauche
	particles.push_back(CreateParticle(scene, Vector3D(halfLen, -halfLen + 2, halfLen), Vector3D(0, 0, 0))); //c�t� arri�re bas-droit

	for (int i = 0; i < 8; i++)
	{
		for (int j = i + 1; j < 8; j++)
		{
			float length = Vector3D::Norm(particles[i]->GetPosition() - particles[j]->GetPosition());
			particleContactSolver.generator.AddParticleLink(new ParticleRod(particles[i], particles[j], length));
		}
	}
}

//##########
//#        #
//#  BLOB  #
//#        #
//##########

float blobMoveX, blobMoveY = 0;
float blobForce = 10;
std::vector<Particle*> blobElements;

void UpdateBlobInput(SDL_Keycode key, bool state)
{
	if (key == SDLK_UP) blobMoveY = state ? 1 : 0;
	else if (key == SDLK_LEFT) blobMoveX = state ? -1 : 0;
	else if (key == SDLK_DOWN) blobMoveY = state ? -1 : 0;
	else if (key == SDLK_RIGHT) blobMoveX = state ? 1 : 0;
}

void UpdateBlobForce()
{
	for (int i = 0; i < blobElements.size(); i++)
		blobElements[i]->AddForce(Vector3D(blobMoveX * blobForce, 0, -blobMoveY * blobForce));
}

void CreateBlob()
{
	for (int i = 0; i < 20; i++)
	{
		Particle* p = CreateParticle(Scene::mainScene, Vector3D(i % 5, 3, i / 5), Vector3D(0, 1, 0), (rand() % 150 + 50) / 200.0f);
		blobElements.push_back(p);

		for (int j = 0; j < blobElements.size(); j++)
		{
			if (i != j && rand() % 10 < 5) 
			{
				Particle* other = blobElements[j];
				float length = Vector3D::Norm(blobElements[i]->GetPosition() - blobElements[j]->GetPosition());
				forcesRegister.AddEntry(p, new ParticleSpringGenerator(other, length * 0.5f));
				forcesRegister.AddEntry(other, new ParticleSpringGenerator(p, length * 0.5f));
				particleContactSolver.generator.AddParticleLink(new ParticleCable(p, other, length));
			}
		}
	}
}

void CreateSnake(Scene* scene, ParticleContactSolver* contactSolver)
{
	//cr�ation des particules
	std::vector<Particle*> particles;
	int nbParticle = 10;
	float size = cbrt(particleMass);
	float spawnX = -6;

	for (int i = 0; i < nbParticle; i++)
	{
		spawnX += size;
		particles.push_back(CreateParticle(scene, Vector3D(spawnX, 1, 0), Vector3D(0, 0, 0)));
		if(i > 0)
			contactSolver->generator.AddParticleLink(new ParticleRod(particles[i], particles[i - 1], size));
	}
	particles[nbParticle-1]->SetVelocity(Vector3D(20, 0, 0));
}

//supprime tout les �l�ments de la sc�ne
void ResetScene(Scene* scene)
{
	particleContactSolver.generator.RemoveAllParticleLink();
	for (int i = Scene::mainScene->gameObjects.size() - 1; i >= 0; i--)
	{
		Scene::mainScene->RemoveParticle(Scene::mainScene->gameObjects[i]);
	}
}

bool mouseButtonDown = false;
int HandleInputs(Renderer* renderer)
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type)
		{
		case SDL_QUIT: //ferme le jeu quand on ferme la fenetre
			runGame = false;
			break;

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_SPACE)
			{
				int pixelMouseX, pixelMouseY;
				SDL_GetMouseState(&pixelMouseX, &pixelMouseY);
				float mouseX = pixelMouseX * 1.0f / SCREEN_WIDTH;
				float mouseY = 1 - (pixelMouseY * 1.0f / SCREEN_HEIGHT);
				float randZ = (-100 + rand() % 200) * randomZDirection * 0.01f;
				CreateParticle(Scene::mainScene, Vector3D(0, 1, 0), Vector3D(-10 + mouseX * 20, mouseY * 15, randZ));
			}
			else
			{
				renderer->camera.UpdateKeyboardInput(event.key.keysym.sym, true);
				UpdateBlobInput(event.key.keysym.sym, true);
			}
			break;

		case SDL_KEYUP:
			renderer->camera.UpdateKeyboardInput(event.key.keysym.sym, false);
			UpdateBlobInput(event.key.keysym.sym, false);
			break;

		case SDL_MOUSEMOTION:
			if(mouseButtonDown)
				renderer->camera.UpdateMouseInput(event.motion.xrel, event.motion.yrel);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_RIGHT)
				mouseButtonDown = true;
			break;

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_RIGHT)
				mouseButtonDown = false;
			break;

		default:
			break;
		}
	}
	return 1;
}

//cr�ation de la fenetre graphique
void MakeImGuiWindow(float physicsUpdateTime)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowSize(ImVec2(0, 0));
	ImGui::SetWindowPos(ImVec2(20, 20));

	ImGui::Text("Particules dans la scene: %d", Scene::mainScene->GetObjectsCount());
	ImGui::Text("Mise a jour de la physique: %.5fms", physicsUpdateTime);
	ImGui::Dummy(ImVec2(0.0f, 20.0f));
	ImGui::PushItemWidth(150);
	ImGui::SliderFloat("Masse des particules", &particleMass, 0.1, 10, "%.2f");
	ImGui::SliderFloat("Dispersion Z", &randomZDirection, 0, 5, "%.1f");

	ImGui::Dummy(ImVec2(0.0f, 20.0f));
	ImGui::PushItemWidth(150);

	ImGui::SliderFloat("Taille du lien", &particleLinkLength, 0, 5, "%.1f");

	if (ImGui::Button("Creer tige", ImVec2(148, 20)))
		CreateRod(Scene::mainScene);
	ImGui::SameLine(160);
	if (ImGui::Button("Creer cable", ImVec2(148, 20)))
		CreateCable(Scene::mainScene);

	if (ImGui::Button("Creer ressort", ImVec2(148, 20)))
		CreateSpring(Scene::mainScene);
	ImGui::SameLine(160);
	if (ImGui::Button("Creer cube de tiges", ImVec2(148, 20)))
		CreateRodCube(Scene::mainScene);

	if (ImGui::Button("Creer blob", ImVec2(300, 20)))
		CreateBlob();

	if (ImGui::Button("Creer serpent", ImVec2(148, 20)))
		CreateSnake(Scene::mainScene, solver);

	ImGui::Dummy(ImVec2(0.0f, 20.0f));
	ImGui::PushItemWidth(150);

	if (ImGui::Button("Reset Scene", ImVec2(300, 20)))
		ResetScene(Scene::mainScene);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main( int argc, char* args[])
{
	//initialisation
	SDL_Window* window = NULL;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Erreur de l'initalisation de SDL : %s\n", SDL_GetError());
	}
	else
	{
		//cr�ation de la fen�tre
		window = SDL_CreateWindow("Sonic Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
		if(window == NULL)
		{
			printf("Erreur de cr�ation de la fen�tre ! SDL_Error: %s\n", SDL_GetError() );
		}
		else //si la fen�tre a �t� cr�e
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui::StyleColorsDark();

			Renderer* renderer = new Renderer(window);
			Scene::mainScene = new Scene(&forcesRegister);

			Uint64 lastUpdate = SDL_GetPerformanceCounter();

			//boucle de jeu
			while (runGame)
			{
				//mise � jour des entr�es
				HandleInputs(renderer);

				//calcul de dt
				Uint64 now = SDL_GetPerformanceCounter();
				float deltaTime = ((now - lastUpdate) / (float)SDL_GetPerformanceFrequency());
				lastUpdate = now;

				//mise � jour de la physique et de la logique
				forcesRegister.Update(deltaTime);
				Scene::mainScene->Update(deltaTime);
				UpdateBlobForce();
				
				//test collisions
				particleContactSolver.UpdateCollisions(Scene::mainScene, 4);
				float physicsUpdateTime = ((SDL_GetPerformanceCounter() - lastUpdate) / (float)SDL_GetPerformanceFrequency()) * 1000;

				renderer->camera.Update(deltaTime);

				//mise � jour de l'affichage
				renderer->Update(Scene::mainScene);
				MakeImGuiWindow(physicsUpdateTime);
				SDL_GL_SwapWindow(window);

				//rendu � 60FPS
				SDL_Delay(1000 / 60);
			}
			
			delete renderer;
			delete Scene::mainScene;
		}
	}

	//destruction de la fen�tre
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
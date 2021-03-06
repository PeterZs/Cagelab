#include "cageOperations.h"

#include <vector>

#include "GUI/qtUtils.h"
#include "common/importFiles.h"
#include "common/exportFiles.h"
#include "controller.h"
#include "drawables/drawableCage.h"
#include "drawables/drawableCharacter.h"
#include "GUI/glCanvas.h"
#include "skinning/noCageSkinning.h"
#include "skinning/meanValueCoordinates.h"
#include "skinning/greenCoordinates.h"
#include "skinning/cageSkinning.h"

void loadCageFromFile()
{
   std::string filename;
   Controller * c = Controller::get();

   if (openFileSelectionDialog(filename, "Load Cage", "3D Meshes (*.off *.obj *.ply)"))
   {

      clearCage();

      std::cout << "Loading cage: " << filename << std::endl;

      std::vector<double> v;
      std::vector<int> f;

      loadMesh(filename.c_str(),v,f);
      DrawableCage * cage = new DrawableCage(v,f);

      c->glCanvas->pushDrawableObject(cage);
      c->glCanvas->pushPickableObject(cage);
      c->cage = cage;
      c->isCageLoaded = true;
   }

   computeCageWeights();

   updateGUI();

}


void saveCageWeightsToFile()
{
   std::string filename;
   Controller * c = Controller::get();

   if (openFileSaveDialog(filename, "Save Weights", "Text File (*.txt)"))
   {
      if(c->areCageWeightsLoaded)
      {
         saveWeights(filename.c_str(), *c->cageWeights);
      }
   }

   updateGUI();

}


void clearCage()
{
   Controller * c = Controller::get();

   if(c->isCageLoaded)
   {
      c->glCanvas->removeDrawableObject(c->cage);
      c->glCanvas->removePickableObject(c->cage);
      delete c->cage;
      c->cage = nullptr;
      c->isCageLoaded = false;
      c->isCageDeformerActive = false;
   }

   if(c->isCageSkinningInitialized)
   {
      delete c->cageSkinning;
      c->cageSkinning = nullptr;
      c->isCageSkinningInitialized = false;
   }

   if(c->areCageWeightsLoaded)
   {
      delete c->mvc;
      delete c->gc;
      c->cageWeights = nullptr;
      c->mvc = nullptr;
      c->gc = nullptr;
      c->areCageWeightsLoaded = false;
   }

   updateGUI();

}

void updateCageInfluenceTexture()
{
   Controller * c = Controller::get();

   if(c->isCageWeightsRenderActive)
   {

      std::vector<float> tex1D(c->character->getNumVertices());

      const std::vector<bool> isVertexSelected = c->cage->getSelectedVertices();

      for(int i=0; i < isVertexSelected.size(); i++)
      {
         if(isVertexSelected[i])
         {
            for(int j=0; j<c->character->getNumVertices();j++){
               tex1D[j] += c->cageWeights->getWeight(i,j);
               if (tex1D[j]>0.99) tex1D[j]=0.99;
               if (tex1D[j]<0.01) tex1D[j]=0.01;
            }
         }
      }
      c->character->updateTexture1D(tex1D);
   }

   updateGUI();

}

void saveCageToFile()
{
   std::string filename;
   Controller * c = Controller::get();

   if (openFileSaveDialog(filename, "Save Cage", "3D Meshes (*.obj *.ply)"))
   {
      if(c->isCageLoaded)
      {
         std::vector<double> v = c->cage->getVerticesVector();
         std::vector<int>    f = c->cage->getTrianglesVector();

         saveMesh(filename.c_str(), v, f);
      }
   }

   updateGUI();

}

void switchCageDeformation()
{
   Controller * c = Controller::get();

   c->isCageDeformerActive = !c->isCageDeformerActive;
   c->cage->activateTransformation = !c->cage->activateTransformation;
   updateGUI();
}

void computeCageWeights()
{
   Controller * c = Controller::get();
   //Compute MeanValueCoordinates
   if(c->isCharacterLoaded && c->isCageLoaded)
   {

      NoCageSkinning * nc = new NoCageSkinning(c->character, c->cage);
      c->nc = nc;

      MeanValueCoordinates * mvc = new MeanValueCoordinates(c->character,
                                                            c->cage);
      c->mvc = mvc;
      //c->cageSkinning = mvc;
      //c->cageWeights = mvc->getWeights();

      GreenCoordinates * gc = new GreenCoordinates(c->character,
                                                   c->cage);
      gc->generateCoords();
      c->gc = gc;

      c->cageWeights = mvc->getWeights();
      c->cageSkinning = mvc;

      c->areCageWeightsLoaded = true;
      c->isCageSkinningInitialized = true;
      c->isCageDeformerActive = true;

   }
}

// factory reset all cameras: returns cameras to original default settings, deletes saved settings, and soft reboots the camera

int main(){
  SystemPtr system = System::GetInstance();
  CameraList camList = system -> GetCameras();
  int numCameras = camList.GetSize();
  CameraPtr pCam = nullptr;
  for (int i = 0; i < numCameras; i++){
    cout << i << endl;
    pCam = camList.GetByIndex(i); 
    pCam -> Init();
    INodeMap& nodeMap = pCam.GetNodeMap();
    CCommandPtr ptrFactoryReset = nodeMap.GetNode("FactoryReset");
    ptrFactoryReset -> Execute();
    cout << i << endl;
  }
  camList.Clear();
  system -> ReleaseInstance();
  
}

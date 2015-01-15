import numpy as np

BL=np.array([-0.977812 ,-0.9425 ])
BL.reshape(1,2)
BR=np.array([0.496562,-0.855])
BR.reshape(1,2)
TL=np.array([-0.962812,0.435 ])
TL.reshape(1,2)
TR=np.array([0.464063,0.4925])
TR.reshape(1,2)

print np.linalg.norm(BR-BL)
print np.linalg.norm(TR-TL)
print TL-BL
print TR-BR
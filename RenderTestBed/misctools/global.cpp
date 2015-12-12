/************************************************************************************************
 *
 *		MTC Project - Client Main Project - ClientGlobalVariables
 *
 *
 *		@desc The global variables for the client.
 *		@author igarciad
 *
 ************************************************************************************************/

#include "global.h"

namespace LC {
	namespace misctools {
		// Global static pointer used to ensure a single instance of the class.
		Global* Global::m_pInstance = NULL;  

		/** This function is called to create an instance of the class. 
		Calling the constructor publicly is not allowed. The constructor 
		is private and is only called by this Instance function.
		*/

		Global* Global::global(){

			if (!m_pInstance)   // Only allow one instance of class to be generated.
				m_pInstance = new Global;

			return m_pInstance;
		}


	}
}
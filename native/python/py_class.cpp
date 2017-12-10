/*****************************************************************************
   Copyright 2004-2008 Steve Menard

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
*****************************************************************************/   

#include <jpype_python.h>

#include <pybind11/pybind11.h>
namespace  py = pybind11;


bool isException(JPClass& clazz)
{
	return JPJni::isThrowable(clazz.getClass());
}

void exportClass(py::module& m) {
	auto ctor = py::init<>();
	py::class_<JPClass>(m, "_JavaClass")
			.def(ctor).
		.def("getName",              &JPClass::getName)
		.def("getBaseClass",         &JPClass::getSuperClass)
		.def("getClassFields",       &JPClass::getClassFields)
		.def("getClassMethods",      &JPClass::getClassMethods)
		.def("newClassInstance",     ctor)

		.def("isInterface", &JPClass::isInterface)
		.def("getBaseInterfaces", &JPClass::getInterfaces)
		.def("isSubclass", &JPClass::isSubclass)
		.def("isPrimitive", &JPClass::isPrimitive)

		.def("isException", &isException)
		.def("isArray", &JPClass::isArray)
		.def("isAbstract", &JPClass::isAbstract)
		.def("getSuperclass",&JPClass::getSuperClass)

		.def("getConstructors", &JPClass::getConstructors)
		.def("getDeclaredConstructors", &JPClass::getDeclaredConstructors)
		.def("getDeclaredFields", &JPClass::getDeclaredFields)
		.def("getDeclaredMethods", &JPClass::getDeclaredMethods)
		.def("getFields", &JPClass::getFields)
		.def("getMethods", &JPClass::getMethods)
		.def("getModifiers", &JPClass::getModifiers);
}

py::list getClassFields(const JPClass& jpclass)
{
	try {
		auto staticFields = jpclass.getStaticFields();
		auto instFields = jpclass.getInstanceFields();

		py::tuple res(staticFields.size()+instFields.size());

		int i = 0;
		for (auto &staticField : staticFields) {
			res[i] = staticField.second;
			i++;
		}

		for (auto &instField : instFields) {
			res[i] = (instField.second);
			i++;
		}


		return res;

	}
	PY_STANDARD_CATCH

	return nullptr;
}

PyObject* PyJPClass::getClassMethods(PyObject* o, PyObject* arg)
{
	JPLocalFrame frame;
	try {
		auto * self = (PyJPClass*)o;

		vector<JPMethod*> methods = self->m_Class->getMethods();

		PyObject* res = JPySequence::newTuple((int)methods.size());

		int i = 0;
		for (auto mth : methods) {

			PyJPMethod* methObj = PyJPMethod::alloc(mth);
			
			JPySequence::setItem(res, i, (PyObject*)methObj);
			i++;
			Py_DECREF(methObj);
		}

		return res;

	}
	PY_STANDARD_CATCH

	return nullptr;
}

PyObject* PyJPClass::newClassInstance(PyObject* o, PyObject* arg)
{
	JPLocalFrame frame;
	try {
		PyJPClass* self = (PyJPClass*)o;
		JPCleaner cleaner;

		//cout << "Creating a new " << self->m_Class->getName().getSimpleName() << endl;
		//JPyHelper::dumpSequenceRefs(arg, "Start");

		vector<HostRef*> args;
		Py_ssize_t len = JPyObject::length(arg);
		for (Py_ssize_t i = 0; i < len; i++)
		{
			PyObject* obj = JPySequence::getItem(arg, i);
			HostRef* ref = new HostRef((void*)obj);
			cleaner.add(ref);
			args.push_back(ref);
			Py_DECREF(obj);
		}

		JPObject* resObject = self->m_Class->newInstance(args);
		PyObject* res = JPyCObject::fromVoidAndDesc((void*)resObject, "JPObject",
													&PythonHostEnvironment::deleteJPObjectDestructor);

		//JPyHelper::dumpSequenceRefs(arg, "End");
		return res;
	}
	PY_STANDARD_CATCH

	return nullptr;
}


bool PyJPClass::check(PyObject* o)
{
	return o->ob_type == &classClassType;
}

PyObject* PyJPClass::getDeclaredMethods(PyObject* o)
{
	JPLocalFrame frame;
	try {
		JPCleaner cleaner;
		PyJPClass* self = (PyJPClass*)o;
		vector<jobject> mth = JPJni::getDeclaredMethods(frame, self->m_Class->getClass());

		PyObject* res = JPySequence::newTuple((int)mth.size());
		JPTypeName methodClassName = JPTypeName::fromSimple("java.lang.reflect.Method");
		JPClass* methodClass = JPTypeManager::findClass(methodClassName);
		for (unsigned int i = 0; i < mth.size(); i++)
		{
			jvalue v;
			v.l = mth[i];
			HostRef* ref = methodClass->asHostObject(v);
			cleaner.add(ref);
			JPySequence::setItem(res, i, (PyObject*)ref->data());
		}

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;	
}

PyObject* PyJPClass::getConstructors(PyObject* o)
{
	JPLocalFrame frame;
	try {
		JPCleaner cleaner;
		PyJPClass* self = (PyJPClass*)o;
		vector<jobject> mth = JPJni::getConstructors(frame, self->m_Class->getClass());

		PyObject* res = JPySequence::newTuple((int)mth.size());
		JPTypeName methodClassName = JPTypeName::fromSimple("java.lang.reflect.Method");
		JPClass* methodClass = JPTypeManager::findClass(methodClassName);
		for (unsigned int i = 0; i < mth.size(); i++)
		{
			jvalue v;
			v.l = mth[i];
			HostRef* ref = methodClass->asHostObject(v);
			cleaner.add(ref);
			JPySequence::setItem(res, i, (PyObject*)ref->data());
		}

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;	
}

PyObject* PyJPClass::getDeclaredConstructors(PyObject* o)
{
	JPLocalFrame frame;
	try {
		JPCleaner cleaner;
		PyJPClass* self = (PyJPClass*)o;
		vector<jobject> mth = JPJni::getDeclaredConstructors(frame, self->m_Class->getClass());

		PyObject* res = JPySequence::newTuple((int)mth.size());
		JPTypeName methodClassName = JPTypeName::fromSimple("java.lang.reflect.Method");
		JPClass* methodClass = JPTypeManager::findClass(methodClassName);
		for (unsigned int i = 0; i < mth.size(); i++)
		{
			jvalue v;
			v.l = mth[i];
			HostRef* ref = methodClass->asHostObject(v);
			cleaner.add(ref);
			JPySequence::setItem(res, i, (PyObject*)ref->data());
		}

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;	
}

PyObject* PyJPClass::getDeclaredFields(PyObject* o)
{
	JPLocalFrame frame;
	try {
		JPCleaner cleaner;
		PyJPClass* self = (PyJPClass*)o;
		vector<jobject> mth = JPJni::getDeclaredFields(frame, self->m_Class->getClass());

		PyObject* res = JPySequence::newTuple((int)mth.size());
		JPTypeName fieldClassName = JPTypeName::fromSimple("java.lang.reflect.Field");
		JPClass* fieldClass = JPTypeManager::findClass(fieldClassName);
		for (unsigned int i = 0; i < mth.size(); i++)
		{
			jvalue v;
			v.l = mth[i];
			HostRef* ref = fieldClass->asHostObject(v);
			cleaner.add(ref);
			JPySequence::setItem(res, i, (PyObject*)ref->data());
		}

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;	
}

PyObject* PyJPClass::getFields(PyObject* o)
{
	JPLocalFrame frame;
	try {
		JPCleaner cleaner;
		PyJPClass* self = (PyJPClass*)o;
		vector<jobject> mth = JPJni::getFields(frame, self->m_Class->getClass());

		PyObject* res = JPySequence::newTuple((int)mth.size());
		JPTypeName fieldClassName = JPTypeName::fromSimple("java.lang.reflect.Field");
		JPClass* fieldClass = JPTypeManager::findClass(fieldClassName);
		for (unsigned int i = 0; i < mth.size(); i++)
		{
			jvalue v;
			v.l = mth[i];
			HostRef* ref = fieldClass->asHostObject(v);
			cleaner.add(ref);
			JPySequence::setItem(res, i, (PyObject*)ref->data());
		}

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;	
}

PyObject* PyJPClass::getModifiers(PyObject* o)
{
	JPLocalFrame frame;
	try {
		PyJPClass* self = (PyJPClass*)o;
		long mod = JPJni::getClassModifiers(self->m_Class->getClass());

		PyObject* res = JPyLong::fromLongLong(mod);

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;	
}

PyObject* PyJPClass::getMethods(PyObject* o)
{
	JPLocalFrame frame;
	try {
		JPCleaner cleaner;
		PyJPClass* self = (PyJPClass*)o;
		vector<jobject> mth = JPJni::getMethods(frame, self->m_Class->getClass());

		PyObject* res = JPySequence::newTuple((int)mth.size());
		JPTypeName methodClassName = JPTypeName::fromSimple("java.lang.reflect.Method");
		JPClass* methodClass = JPTypeManager::findClass(methodClassName);
		for (unsigned int i = 0; i < mth.size(); i++)
		{
			jvalue v;
			v.l = mth[i];
			HostRef* ref = methodClass->asHostObject(v);
			cleaner.add(ref);
			JPySequence::setItem(res, i, (PyObject*)ref->data());
		}

		return res;
	}
	PY_STANDARD_CATCH;
	return NULL;
	
}


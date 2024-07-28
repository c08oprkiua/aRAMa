#ifndef ARAMA_NODE_LIST_H
#define ARAMA_NODE_LIST_H

template<typename DataType>
class Node {
public:
	DataType *data;
	Node *next;
};

template<typename Data>
class NodeList {
private:
	Node<Data> *list;
public:
	~NodeList(){
		destroy();
	}

	void destroy(){
		Node<Data> *currentNode = list;
		while (currentNode != NULL) {
			Node<Data> *previousNode = currentNode;
			currentNode = currentNode->next;
            delete previousNode;
		}
	}
	
	void insert(Data data){
		Node<Data> *addedNode = new Node<Data>;

		addedNode->data = data;
		addedNode->next = list;
		list = addedNode;
	}

	int length(){
		int length = 0;
		Node<Data> *current;
		for (current = list; current != NULL; current = current->next) {
			length++;
		}
		return length;
	}

	void reverse(){
		Node<Data> *previous = NULL;
		Node<Data> *current = list;
		Node<Data> *next;

		while (current != NULL){
			next = current->next;
			current->next = previous;
			previous = current;
			current = next;
		}

		list = previous;
	}

	Node<Data> *get_list(){
		return list;
	}

};

#endif
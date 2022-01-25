#include <iostream>
#include <assert.h>
#include <cmath>

#define ON 1
#define INSERTION_BALANCE ON	// ���� �뷱�� �������

#if INSERTION_BALANCE == ON
#define DELETION_BALANCE ON		// ���� �뷱�� �������
#endif

#define DeleteSafe(x)			\
do {							\
	if (x) {					\
		delete x;				\
	}							\
	x = nullptr;				\
} while (0);	

namespace AVLMemo {

struct Node
{
	int Data = 0;
	int Height = 0;
	Node* Left = nullptr;
	Node* Right = nullptr;
	Node* Parent = nullptr;

	Node* Any() {
		if (Left) {
			return Left;
		}

		if (Right) {
			return Right;
		}

		return nullptr;
	}

	int ChildCount() {
		if (Left && Right) {
			return 2;
		}

		if (Left || Right) {
			return 1;
		}

		return 0;
	}

	void DeleteChild(Node* node) {
		if (Left == node) {
			DeleteSafe(Left);
		} else if (Right == node) {
			DeleteSafe(Right);
		} else {
			assert("����?");
		}
	}

	void UpdateHeight() {
		int iLeftHeight = LeftSubtreeHeight();
		int iRightHeight = RightSubtreeHeight();
		
		this->Height = std::max(iLeftHeight, iRightHeight) + 1;
	}

	int LeftSubtreeHeight() {
		return Left ? Left->Height : 0;
	}

	int RightSubtreeHeight() {
		return Right ? Right->Height : 0;
	}
};

class AVLTree
{
public:
	AVLTree() {
		m_pVirtualRoot = new Node;
		m_iSize = 0;
	}
	~AVLTree() {
		Clear();
		DeleteSafe(m_pVirtualRoot);
	}
public:
	void Add(int data) {
		if (m_iSize == 0) {
			m_pVirtualRoot->Left = new Node{ data };
			m_pVirtualRoot->Left->Parent = m_pVirtualRoot;
			m_iSize++;
			return;
		}

		Node* pRootNode = m_pVirtualRoot->Any();
		Node* pNewNode = Add(m_pVirtualRoot->Any(), data);
	#if INSERTION_BALANCE == ON
		BalanceStart(pNewNode);
	#endif
		m_iSize++;
	}

	bool Remove(int data) {
		Node* pDel = FindNode(data);	// ������ ��带 ã�´�.

		// ������ ��带 ��ã�� ���
		if (pDel == nullptr) {
			return false;
		}

		Node* pBalanceStartingNode = nullptr;	// �뷱���� ������ ��� = ���������� ������ ����� �θ� ���
		int iChildCount = pDel->ChildCount();

		if (iChildCount == 0) {
			pBalanceStartingNode = RemoveDelNoChild(pDel);
		} else if (iChildCount == 1) {
			pBalanceStartingNode = RemoveDelOneChild(pDel);
		} else {
			pBalanceStartingNode = RemoveDelTwoChild(pDel);
		}

	#if DELETION_BALANCE == ON
		if (pBalanceStartingNode)
			BalanceStart(pBalanceStartingNode);
	#endif
		m_iSize--;
		return true;
	}

	bool Find(int data) {
		return FindNode(data) != nullptr;
	}

	
	void Clear() {
		Node* pRoot = m_pVirtualRoot->Any();

		if (pRoot == nullptr) {
			return;
		}

		DeleteElementRecursive(pRoot->Left);
		DeleteElementRecursive(pRoot->Right);
		DeleteSafe(pRoot);
	}
private:
	void DeleteElementRecursive(Node* node) {
		if (node == nullptr) {
			return;
		}

		DeleteElementRecursive(node->Left);
		DeleteElementRecursive(node->Right);
		DeleteSafe(node);
	}


	Node* Add(Node* root, int data) {
		Node* pParent = nullptr;
		Node* pCur = root;
		Node* pNewNode = new Node{ data };
		int iComp = 0;

		while (pCur != nullptr) {
			iComp = Compare(pCur->Data, data);
			pParent = pCur;
			pCur = iComp > 0 ? pCur->Right : pCur->Left;
		}

		if (iComp <= 0) {
			pParent->Left = pNewNode;
			pParent->Left->Parent = pParent;
		} else {
			pParent->Right = pNewNode;
			pParent->Right->Parent = pParent;
		}

		return pNewNode;
	}

	Node* RemoveDelNoChild(Node* del) {
		Node* pParent = del->Parent;
		pParent->DeleteChild(del);
		return pParent;
	}

	Node* RemoveDelOneChild(Node* del) {
		Node* pParent = del->Parent;
		Node* pChild = del->Any();
		Node* pBalanceStartingNode = nullptr;


		if (pParent == m_pVirtualRoot) {
			pParent->Left = pChild;
		} else {
			if (pParent->Right == del) {
				pParent->Right = pChild;
			} else {
				pParent->Left = pChild;
			}
			pBalanceStartingNode = pParent;
		}

		pChild->Parent = pParent;
		DeleteSafe(del);
		return pBalanceStartingNode;
	}

	Node* RemoveDelTwoChild(Node* del) {
		Node* pSmallestParent = del;		// ������ ����� �θ�
		Node* pSmallest = del->Right;		// ������ ���

		while (pSmallest->Left != nullptr) {
			pSmallest = pSmallest->Left;
		}

		pSmallestParent = pSmallest->Parent;
		del->Data = pSmallest->Data;

		if (pSmallestParent->Right == pSmallest) {
			pSmallestParent->Right = pSmallest->Right;
			if (pSmallest->Right)
				pSmallest->Right->Parent = pSmallestParent;
		} else {
			pSmallestParent->Left = pSmallest->Right;
			if (pSmallest->Right)
				pSmallest->Right->Parent = pSmallestParent;
		}
		DeleteSafe(pSmallest);
		return pSmallestParent;
	}

	Node* FindNode(int data) {
		Node* pCur = m_pVirtualRoot->Any();

		while (pCur != nullptr) {
			if (pCur->Data == data) {
				return pCur;
			}

			if (pCur->Data < data) {
				pCur = pCur->Right;
			} else {
				pCur = pCur->Left;
			}
		}

		return nullptr;
	}

	void BalanceStart(Node* node) {
		Node* pCur = node;

		while (pCur != m_pVirtualRoot) {
			pCur->UpdateHeight();
			Balance(pCur);
			pCur = pCur->Parent;
		}
	}

	// �ش� ��� �������� �뷱�� ����
	void Balance(Node* node) {
		int iHeightDiff = GetHeightDiff(node);
		if (iHeightDiff >= 2) {

			int iChildHeightDiff = GetHeightDiff(node->Right);
			if (iChildHeightDiff >= 1) {
				RotateRR(node);
			} else {
				RotateRL(node);
			}
		}
		else if (iHeightDiff <= -2) {
			int iChildHeightDiff = GetHeightDiff(node->Left);
			if (iChildHeightDiff <= -1) {
				RotateLL(node);
			} else {
				RotateLR(node);
			}
		}
	}

	int GetHeightDiff(Node* node) {
		return node->RightSubtreeHeight() - node->LeftSubtreeHeight();
	}


	void RotateRR(Node* cur) {
		Node* pParent = cur->Parent;
		Node* pCur = cur;
		Node* pChild = cur->Right;
		Node* pChildLeft = cur->Right->Left;

		if (pParent->Left == pCur) 
			pParent->Left = pChild;
		else
			pParent->Right = pChild;
		pChild->Parent = pParent;


		pCur->Right = pChildLeft;
		if (pChildLeft)
			pChildLeft->Parent = pCur;

		pChild->Left = pCur;
		pCur->Parent = pChild;

		
	#if INSERTION_BALANCE == ON
		// ȸ�� �� ���̸� �������ش�.
		pCur->UpdateHeight();
		pChild->UpdateHeight();
	#endif
	}

	void RotateLR(Node* cur) {
		RotateRR(cur->Left);
		RotateLL(cur);
	}

	void RotateRL(Node* cur) {
		RotateLL(cur->Right);
		RotateRR(cur);
	}

	void RotateLL(Node* cur) {
		Node* pParent = cur->Parent;
		Node* pCur = cur;
		Node* pChild = cur->Left;
		Node* pChildRight = cur->Left->Right;

		if (pParent->Left == pCur)
			pParent->Left = pChild;
		else
			pParent->Right = pChild;
		pChild->Parent = pParent;

		pCur->Left = pChildRight;
		if (pChildRight)
			pChildRight->Parent = pCur;

		pChild->Right = pCur;
		pCur->Parent = pChild;

		// ȸ�� �� ���̸� �������ش�.
	#if INSERTION_BALANCE == ON
		pCur->UpdateHeight();
		pChild->UpdateHeight();
	#endif
	}


	int Compare(int src, int dst) {
		if (src < dst) {
			return 1;
		}

		if (src > dst) {
			return -1;
		}

		return 0;
	}

	
private:
	Node* m_pVirtualRoot;
	int m_iSize;
};

} // namespace AVLMemo


int main() {
	AVLMemo::AVLTree tree;

	tree.Add(1);
	tree.Add(2);
	tree.Add(3);
	tree.Add(4);
	tree.Add(5);

	std::cout << std::boolalpha;
	std::cout << "1 ã�� : " << tree.Find(1) << "\n";
	std::cout << "2 ã�� : " << tree.Find(2) << "\n";
	std::cout << "3 ã�� : " << tree.Find(3) << "\n";
	std::cout << "4 ã�� : " << tree.Find(4) << "\n";
	std::cout << "5 ã�� : " << tree.Find(5) << "\n";

	tree.Remove(1);
	tree.Remove(2);
	tree.Remove(3);
	tree.Remove(4);
	tree.Remove(5);
}
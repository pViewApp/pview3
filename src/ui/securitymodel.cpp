#include "securitymodel.h"

QVariant pvui::models::SecurityModel::data(const QModelIndex& index, int role) const
{
	if (role != Qt::DisplayRole || index.internalPointer() == nullptr || !index.isValid()) return QVariant();

	pv::Security* security = static_cast<pv::Security*>(index.internalPointer());
	switch (index.column()) {
	case 0:
		return QString::fromStdString(security->symbol());
	case 1:
		return QString::fromStdString(security->name());
	case 2:
		return QString::fromStdString(security->assetClass());
	case 3:
		return QString::fromStdString(security->sector());
	default:
		return QVariant();
	}
}
